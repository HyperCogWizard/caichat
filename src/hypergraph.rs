//! Hypergraph synergy and coordination module for CAIChat
//!
//! This module provides the core functionality for auditing and reinforcing
//! module interconnections through hypergraph-based memory management and
//! cognitive coherence validation.

use crate::config::GlobalConfig;
use anyhow::{anyhow, Result};
use indexmap::IndexMap;
use parking_lot::RwLock;
use std::collections::{HashMap, HashSet};
use std::sync::Arc;
use std::time::{Duration, Instant};

/// Represents the health and synergy status of core modules
#[derive(Debug, Clone)]
pub struct ModuleAudit {
    pub module_name: String,
    pub status: ModuleStatus,
    pub synergy_score: f64,
    pub hypergraph_connections: usize,
    pub last_checked: Instant,
    pub issues: Vec<String>,
    pub recommendations: Vec<String>,
}

#[derive(Debug, Clone)]
pub enum ModuleStatus {
    Healthy,
    Warning,
    Critical,
    Disconnected,
}

/// Manages hypergraph synergy across all core modules
pub struct HypergraphCoordinator {
    config: GlobalConfig,
    module_registry: RwLock<IndexMap<String, ModuleMetrics>>,
    synergy_matrix: RwLock<HashMap<(String, String), f64>>,
    audit_history: RwLock<Vec<ModuleAudit>>,
    performance_metrics: RwLock<PerformanceMetrics>,
}

#[derive(Debug, Clone)]
struct ModuleMetrics {
    name: String,
    active_connections: HashSet<String>,
    message_count: u64,
    error_count: u64,
    last_activity: Instant,
    memory_usage: usize,
    cognitive_load: f64,
}

#[derive(Debug, Clone, Default)]
pub struct PerformanceMetrics {
    pub total_operations: u64,
    pub average_response_time: Duration,
    pub memory_efficiency: f64,
    pub synergy_coefficient: f64,
}

impl HypergraphCoordinator {
    /// Create a new hypergraph coordinator instance
    pub fn new(config: GlobalConfig) -> Self {
        Self {
            config,
            module_registry: RwLock::new(IndexMap::new()),
            synergy_matrix: RwLock::new(HashMap::new()),
            audit_history: RwLock::new(Vec::new()),
            performance_metrics: RwLock::new(PerformanceMetrics::default()),
        }
    }

    /// Register a core module for hypergraph tracking
    pub fn register_module(&self, module_name: &str) -> Result<()> {
        let mut registry = self.module_registry.write();
        
        let metrics = ModuleMetrics {
            name: module_name.to_string(),
            active_connections: HashSet::new(),
            message_count: 0,
            error_count: 0,
            last_activity: Instant::now(),
            memory_usage: 0,
            cognitive_load: 0.0,
        };
        
        registry.insert(module_name.to_string(), metrics);
        
        log::info!("Registered module '{}' for hypergraph coordination", module_name);
        Ok(())
    }

    /// Establish a synergy connection between two modules
    pub fn establish_connection(&self, module_a: &str, module_b: &str, strength: f64) -> Result<()> {
        let mut registry = self.module_registry.write();
        let mut synergy_matrix = self.synergy_matrix.write();
        
        // Update module connections
        if let Some(metrics_a) = registry.get_mut(module_a) {
            metrics_a.active_connections.insert(module_b.to_string());
        }
        if let Some(metrics_b) = registry.get_mut(module_b) {
            metrics_b.active_connections.insert(module_a.to_string());
        }
        
        // Update synergy matrix
        synergy_matrix.insert((module_a.to_string(), module_b.to_string()), strength);
        synergy_matrix.insert((module_b.to_string(), module_a.to_string()), strength);
        
        log::debug!("Established connection: {} <-> {} (strength: {:.2})", 
                   module_a, module_b, strength);
        Ok(())
    }

    /// Record module activity for hypergraph analysis
    pub fn record_activity(&self, module_name: &str, operation_type: &str, duration: Duration) -> Result<()> {
        let mut registry = self.module_registry.write();
        
        if let Some(metrics) = registry.get_mut(module_name) {
            metrics.message_count += 1;
            metrics.last_activity = Instant::now();
            
            // Update cognitive load based on operation type and duration
            let load_factor = match operation_type {
                "llm_completion" => 0.8,
                "embedding" => 0.5,
                "session_management" => 0.3,
                "rag_query" => 0.6,
                "hypergraph_update" => 0.9,
                _ => 0.4,
            };
            
            metrics.cognitive_load = (metrics.cognitive_load * 0.9) + 
                                   (load_factor * duration.as_secs_f64() * 0.1);
        }
        
        // Update global performance metrics
        let mut perf = self.performance_metrics.write();
        perf.total_operations += 1;
        
        // Exponential moving average for response time
        let alpha = 0.1;
        let current_avg = perf.average_response_time.as_secs_f64();
        let new_avg = current_avg * (1.0 - alpha) + duration.as_secs_f64() * alpha;
        perf.average_response_time = Duration::from_secs_f64(new_avg);
        
        Ok(())
    }

    /// Record an error for error tracking and module health assessment
    pub fn record_error(&self, module_name: &str, error: &str) -> Result<()> {
        let mut registry = self.module_registry.write();
        
        if let Some(metrics) = registry.get_mut(module_name) {
            metrics.error_count += 1;
            log::warn!("Module '{}' error: {}", module_name, error);
        }
        
        Ok(())
    }

    /// Perform comprehensive audit of all core modules
    pub fn audit_core_modules(&self) -> Result<Vec<ModuleAudit>> {
        let registry = self.module_registry.read();
        let synergy_matrix = self.synergy_matrix.read();
        let mut audits = Vec::new();
        
        for (module_name, metrics) in registry.iter() {
            let mut issues = Vec::new();
            let mut recommendations = Vec::new();
            
            // Calculate synergy score
            let synergy_score = self.calculate_synergy_score(module_name, &synergy_matrix);
            
            // Determine module status based on various factors
            let status = if metrics.error_count > 10 {
                issues.push("High error count detected".to_string());
                recommendations.push("Review error handling and add circuit breakers".to_string());
                ModuleStatus::Critical
            } else if metrics.active_connections.is_empty() {
                issues.push("Module appears disconnected from hypergraph".to_string());
                recommendations.push("Establish connections with related modules".to_string());
                ModuleStatus::Disconnected
            } else if synergy_score < 0.5 {
                issues.push("Low synergy score with other modules".to_string());
                recommendations.push("Improve inter-module communication patterns".to_string());
                ModuleStatus::Warning
            } else {
                ModuleStatus::Healthy
            };
            
            // Check for stale activity
            if metrics.last_activity.elapsed() > Duration::from_secs(300) {
                issues.push("No recent activity detected".to_string());
                recommendations.push("Verify module is active and responding".to_string());
            }
            
            // Check cognitive load
            if metrics.cognitive_load > 0.9 {
                issues.push("High cognitive load detected".to_string());
                recommendations.push("Consider load balancing or resource optimization".to_string());
            }
            
            let audit = ModuleAudit {
                module_name: module_name.clone(),
                status,
                synergy_score,
                hypergraph_connections: metrics.active_connections.len(),
                last_checked: Instant::now(),
                issues,
                recommendations,
            };
            
            audits.push(audit);
        }
        
        // Store audit history
        let mut history = self.audit_history.write();
        history.extend(audits.clone());
        
        // Keep only last 100 audits per module
        if history.len() > 1000 {
            history.drain(0..500);
        }
        
        Ok(audits)
    }

    /// Calculate synergy score for a specific module
    fn calculate_synergy_score(&self, module_name: &str, synergy_matrix: &HashMap<(String, String), f64>) -> f64 {
        let registry = self.module_registry.read();
        
        if let Some(metrics) = registry.get(module_name) {
            let connection_count = metrics.active_connections.len() as f64;
            let max_connections = registry.len() as f64 - 1.0; // Exclude self
            
            if max_connections == 0.0 {
                return 1.0; // Single module case
            }
            
            // Calculate average connection strength
            let total_strength: f64 = metrics.active_connections.iter()
                .filter_map(|connected_module| {
                    synergy_matrix.get(&(module_name.to_string(), connected_module.clone()))
                })
                .sum();
            
            let average_strength = if connection_count > 0.0 {
                total_strength / connection_count
            } else {
                0.0
            };
            
            // Combine connectivity and strength factors
            let connectivity_factor = connection_count / max_connections;
            let strength_factor = average_strength;
            
            // Weight connectivity and strength equally
            (connectivity_factor * 0.5) + (strength_factor * 0.5)
        } else {
            0.0
        }
    }

    /// Get current performance metrics
    pub fn get_performance_metrics(&self) -> PerformanceMetrics {
        let perf = self.performance_metrics.read();
        let registry = self.module_registry.read();
        let synergy_matrix = self.synergy_matrix.read();
        
        // Calculate overall synergy coefficient
        let synergy_coefficient = if !registry.is_empty() {
            registry.keys()
                .map(|module| self.calculate_synergy_score(module, &synergy_matrix))
                .sum::<f64>() / registry.len() as f64
        } else {
            0.0
        };
        
        PerformanceMetrics {
            total_operations: perf.total_operations,
            average_response_time: perf.average_response_time,
            memory_efficiency: self.calculate_memory_efficiency(),
            synergy_coefficient,
        }
    }

    /// Calculate memory efficiency across all modules
    fn calculate_memory_efficiency(&self) -> f64 {
        let registry = self.module_registry.read();
        
        let total_memory: usize = registry.values().map(|m| m.memory_usage).sum();
        let total_operations: u64 = registry.values().map(|m| m.message_count).sum();
        
        if total_operations > 0 {
            1.0 / (1.0 + (total_memory as f64 / total_operations as f64 / 1000.0))
        } else {
            1.0
        }
    }

    /// Generate a comprehensive system health report
    pub fn generate_health_report(&self) -> Result<String> {
        let audits = self.audit_core_modules()?;
        let metrics = self.get_performance_metrics();
        
        let mut report = String::new();
        
        report.push_str("╔══════════════════════════════════════════════════════════════╗\n");
        report.push_str("║                  Hypergraph Synergy Report                  ║\n");
        report.push_str("╚══════════════════════════════════════════════════════════════╝\n\n");
        
        // Overall metrics
        report.push_str(&format!("📊 Performance Metrics:\n"));
        report.push_str(&format!("   Total Operations: {}\n", metrics.total_operations));
        report.push_str(&format!("   Average Response Time: {:.2}ms\n", 
                                metrics.average_response_time.as_millis()));
        report.push_str(&format!("   Memory Efficiency: {:.2}%\n", metrics.memory_efficiency * 100.0));
        report.push_str(&format!("   Synergy Coefficient: {:.2}%\n\n", metrics.synergy_coefficient * 100.0));
        
        // Module status summary
        let healthy_count = audits.iter().filter(|a| matches!(a.status, ModuleStatus::Healthy)).count();
        let warning_count = audits.iter().filter(|a| matches!(a.status, ModuleStatus::Warning)).count();
        let critical_count = audits.iter().filter(|a| matches!(a.status, ModuleStatus::Critical)).count();
        let disconnected_count = audits.iter().filter(|a| matches!(a.status, ModuleStatus::Disconnected)).count();
        
        report.push_str(&format!("🔍 Module Status Summary:\n"));
        report.push_str(&format!("   ✅ Healthy: {}\n", healthy_count));
        report.push_str(&format!("   ⚠️  Warning: {}\n", warning_count));
        report.push_str(&format!("   🚨 Critical: {}\n", critical_count));
        report.push_str(&format!("   🔌 Disconnected: {}\n\n", disconnected_count));
        
        // Detailed module reports
        report.push_str("📋 Detailed Module Analysis:\n\n");
        for audit in &audits {
            let status_emoji = match audit.status {
                ModuleStatus::Healthy => "✅",
                ModuleStatus::Warning => "⚠️",
                ModuleStatus::Critical => "🚨",
                ModuleStatus::Disconnected => "🔌",
            };
            
            report.push_str(&format!("{} {} (Synergy: {:.2}%, Connections: {})\n", 
                                   status_emoji, audit.module_name, 
                                   audit.synergy_score * 100.0, audit.hypergraph_connections));
            
            if !audit.issues.is_empty() {
                report.push_str("   Issues:\n");
                for issue in &audit.issues {
                    report.push_str(&format!("   • {}\n", issue));
                }
            }
            
            if !audit.recommendations.is_empty() {
                report.push_str("   Recommendations:\n");
                for rec in &audit.recommendations {
                    report.push_str(&format!("   → {}\n", rec));
                }
            }
            
            report.push_str("\n");
        }
        
        Ok(report)
    }
}

/// Global instance for hypergraph coordination
static HYPERGRAPH_COORDINATOR: once_cell::sync::OnceCell<Arc<HypergraphCoordinator>> = once_cell::sync::OnceCell::new();

/// Initialize the global hypergraph coordinator
pub fn init_hypergraph_coordinator(config: GlobalConfig) -> Result<()> {
    let coordinator = Arc::new(HypergraphCoordinator::new(config));
    
    HYPERGRAPH_COORDINATOR.set(coordinator)
        .map_err(|_| anyhow!("Hypergraph coordinator already initialized"))?;
    
    Ok(())
}

/// Get the global hypergraph coordinator instance
pub fn get_hypergraph_coordinator() -> Result<Arc<HypergraphCoordinator>> {
    HYPERGRAPH_COORDINATOR.get()
        .cloned()
        .ok_or_else(|| anyhow!("Hypergraph coordinator not initialized"))
}

/// Convenience function to register a module
pub fn register_module(module_name: &str) -> Result<()> {
    get_hypergraph_coordinator()?.register_module(module_name)
}

/// Convenience function to establish module connections
pub fn establish_connection(module_a: &str, module_b: &str, strength: f64) -> Result<()> {
    get_hypergraph_coordinator()?.establish_connection(module_a, module_b, strength)
}

/// Convenience function to record module activity
pub fn record_activity(module_name: &str, operation_type: &str, duration: Duration) -> Result<()> {
    get_hypergraph_coordinator()?.record_activity(module_name, operation_type, duration)
}

/// Convenience function to record errors
pub fn record_error(module_name: &str, error: &str) -> Result<()> {
    get_hypergraph_coordinator()?.record_error(module_name, error)
}

/// Convenience function to audit core modules
pub fn audit_core_modules() -> Result<Vec<ModuleAudit>> {
    get_hypergraph_coordinator()?.audit_core_modules()
}

/// Convenience function to generate health report
pub fn generate_health_report() -> Result<String> {
    get_hypergraph_coordinator()?.generate_health_report()
}