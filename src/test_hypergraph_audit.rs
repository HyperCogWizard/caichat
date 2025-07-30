//! Test suite for hypergraph audit and reinforcement functionality
//!
//! This module tests the core hypergraph synergy features to ensure
//! proper module coordination and health monitoring.

#[cfg(test)]
mod tests {
    use crate::config::GlobalConfig;
    use crate::hypergraph::*;
    use std::sync::Arc;
    use std::time::Duration;
    use parking_lot::RwLock;

    fn create_test_config() -> GlobalConfig {
        // Create a minimal test configuration
        Arc::new(RwLock::new(crate::config::Config::default()))
    }

    #[tokio::test]
    async fn test_hypergraph_coordinator_initialization() {
        let config = create_test_config();
        let coordinator = HypergraphCoordinator::new(config);
        
        // Test basic coordinator creation
        assert!(coordinator.module_registry.read().is_empty());
        assert!(coordinator.synergy_matrix.read().is_empty());
    }

    #[tokio::test]
    async fn test_module_registration() {
        let config = create_test_config();
        let coordinator = HypergraphCoordinator::new(config);
        
        // Register some test modules
        coordinator.register_module("test_client").unwrap();
        coordinator.register_module("test_config").unwrap();
        coordinator.register_module("test_rag").unwrap();
        
        let registry = coordinator.module_registry.read();
        assert_eq!(registry.len(), 3);
        assert!(registry.contains_key("test_client"));
        assert!(registry.contains_key("test_config"));
        assert!(registry.contains_key("test_rag"));
    }

    #[tokio::test]
    async fn test_connection_establishment() {
        let config = create_test_config();
        let coordinator = HypergraphCoordinator::new(config);
        
        // Register modules
        coordinator.register_module("module_a").unwrap();
        coordinator.register_module("module_b").unwrap();
        
        // Establish connection
        coordinator.establish_connection("module_a", "module_b", 0.8).unwrap();
        
        let registry = coordinator.module_registry.read();
        let synergy_matrix = coordinator.synergy_matrix.read();
        
        // Check connections are recorded
        assert!(registry.get("module_a").unwrap().active_connections.contains("module_b"));
        assert!(registry.get("module_b").unwrap().active_connections.contains("module_a"));
        
        // Check synergy matrix
        assert_eq!(synergy_matrix.get(&("module_a".to_string(), "module_b".to_string())), Some(&0.8));
        assert_eq!(synergy_matrix.get(&("module_b".to_string(), "module_a".to_string())), Some(&0.8));
    }

    #[tokio::test]
    async fn test_activity_recording() {
        let config = create_test_config();
        let coordinator = HypergraphCoordinator::new(config);
        
        // Register a module
        coordinator.register_module("test_module").unwrap();
        
        // Record some activities
        coordinator.record_activity("test_module", "llm_completion", Duration::from_millis(100)).unwrap();
        coordinator.record_activity("test_module", "embedding", Duration::from_millis(50)).unwrap();
        
        let registry = coordinator.module_registry.read();
        let metrics = registry.get("test_module").unwrap();
        
        assert_eq!(metrics.message_count, 2);
        assert!(metrics.cognitive_load > 0.0);
    }

    #[tokio::test]
    async fn test_error_recording() {
        let config = create_test_config();
        let coordinator = HypergraphCoordinator::new(config);
        
        // Register a module
        coordinator.register_module("test_module").unwrap();
        
        // Record some errors
        coordinator.record_error("test_module", "Test error 1").unwrap();
        coordinator.record_error("test_module", "Test error 2").unwrap();
        
        let registry = coordinator.module_registry.read();
        let metrics = registry.get("test_module").unwrap();
        
        assert_eq!(metrics.error_count, 2);
    }

    #[tokio::test]
    async fn test_core_module_audit() {
        let config = create_test_config();
        let coordinator = HypergraphCoordinator::new(config);
        
        // Set up a test scenario
        coordinator.register_module("healthy_module").unwrap();
        coordinator.register_module("warning_module").unwrap();
        coordinator.register_module("disconnected_module").unwrap();
        
        // Create some connections
        coordinator.establish_connection("healthy_module", "warning_module", 0.9).unwrap();
        
        // Record activities
        coordinator.record_activity("healthy_module", "llm_completion", Duration::from_millis(50)).unwrap();
        coordinator.record_activity("warning_module", "embedding", Duration::from_millis(200)).unwrap();
        
        // Record errors for warning module
        for i in 0..5 {
            coordinator.record_error("warning_module", &format!("Warning error {}", i)).unwrap();
        }
        
        // Perform audit
        let audits = coordinator.audit_core_modules().unwrap();
        
        assert_eq!(audits.len(), 3);
        
        // Check audit results
        let healthy_audit = audits.iter().find(|a| a.module_name == "healthy_module").unwrap();
        let warning_audit = audits.iter().find(|a| a.module_name == "warning_module").unwrap();
        let disconnected_audit = audits.iter().find(|a| a.module_name == "disconnected_module").unwrap();
        
        assert!(matches!(healthy_audit.status, ModuleStatus::Healthy));
        assert!(matches!(warning_audit.status, ModuleStatus::Warning));
        assert!(matches!(disconnected_audit.status, ModuleStatus::Disconnected));
        
        assert!(healthy_audit.synergy_score > 0.0);
        assert_eq!(disconnected_audit.hypergraph_connections, 0);
    }

    #[tokio::test]
    async fn test_performance_metrics() {
        let config = create_test_config();
        let coordinator = HypergraphCoordinator::new(config);
        
        // Register modules and record activities
        coordinator.register_module("module1").unwrap();
        coordinator.register_module("module2").unwrap();
        coordinator.establish_connection("module1", "module2", 0.8).unwrap();
        
        coordinator.record_activity("module1", "llm_completion", Duration::from_millis(100)).unwrap();
        coordinator.record_activity("module2", "embedding", Duration::from_millis(50)).unwrap();
        
        let metrics = coordinator.get_performance_metrics();
        
        assert!(metrics.total_operations > 0);
        assert!(metrics.synergy_coefficient > 0.0);
        assert!(metrics.memory_efficiency > 0.0);
    }

    #[tokio::test]
    async fn test_health_report_generation() {
        let config = create_test_config();
        let coordinator = HypergraphCoordinator::new(config);
        
        // Set up test scenario
        coordinator.register_module("test_module").unwrap();
        coordinator.record_activity("test_module", "llm_completion", Duration::from_millis(100)).unwrap();
        
        let report = coordinator.generate_health_report().unwrap();
        
        // Basic checks that report is generated
        assert!(!report.is_empty());
        assert!(report.contains("Hypergraph Synergy Report"));
        assert!(report.contains("Performance Metrics"));
        assert!(report.contains("test_module"));
    }

    #[tokio::test]
    async fn test_global_functions() {
        let config = create_test_config();
        
        // Initialize global coordinator
        init_hypergraph_coordinator(config).unwrap();
        
        // Test global convenience functions
        register_module("global_test_module").unwrap();
        establish_connection("global_test_module", "global_test_module", 0.5).unwrap();
        record_activity("global_test_module", "test_operation", Duration::from_millis(10)).unwrap();
        record_error("global_test_module", "test error").unwrap();
        
        // Audit modules
        let audits = audit_core_modules().unwrap();
        assert!(!audits.is_empty());
        
        // Generate health report
        let report = generate_health_report().unwrap();
        assert!(!report.is_empty());
    }
}