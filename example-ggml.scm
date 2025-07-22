#!/usr/bin/env guile
!#

;;
;; example-ggml.scm - Example showcasing GGML OpenCog integration
;;

;; This example demonstrates the new GGML integration features
;; for running local models with OpenCog cognitive architecture

(display "╔══════════════════════════════════════════════════════════════╗\n")
(display "║              CAIChat GGML Integration Example               ║\n")
(display "╚══════════════════════════════════════════════════════════════╝\n\n")

(display "This example shows how to use CAIChat with GGML for local model inference\n")
(display "integrated with OpenCog's cognitive architecture.\n\n")

;; Load the necessary modules (when OpenCog is available)
(display "Example 1: GGML Configuration\n")
(display "==============================\n")
(display "(add-to-load-path \".\")\n")
(display "(use-modules (opencog caichat config))\n\n")

(display ";; Setup GGML with a local model\n")
(display "(caichat-setup-ggml \"/path/to/llama2-7b-chat.ggmlv3.q4_0.bin\" 4 2048)\n\n")

(display "Example 2: Basic GGML Usage\n")
(display "============================\n")
(display ";; Create a GGML session\n")
(display "(define ggml-session (caichat-ggml-session))\n\n")

(display ";; Simple chat with local model\n")
(display "(caichat-ggml-chat \"Hello, explain what makes GGML special\")\n\n")

(display ";; Check model status\n")
(display "(caichat-ggml-model-status)\n\n")

(display "Example 3: Cognitive Architecture Integration\n")
(display "=============================================\n")
(display ";; Create atoms for cognitive context\n")
(display "(define ai-concept (Concept \"artificial-intelligence\"))\n")
(display "(define ml-concept (Concept \"machine-learning\"))\n\n")

(display ";; Ask about specific atoms using GGML\n")
(display "(caichat-ggml-ask-with-atom ai-concept \n")
(display "  \"What are the key principles of artificial intelligence?\")\n\n")

(display ";; Use cognitive completion with context\n")
(display "(caichat-ggml-cognitive-chat \n")
(display "  \"Explain neural networks\" \n")
(display "  ml-concept)\n\n")

(display "Example 4: Neural-Symbolic Bridge\n")
(display "==================================\n")
(display ";; Perform neural-symbolic analysis\n")
(display "(caichat-neural-symbolic-analysis \n")
(display "  \"consciousness and artificial intelligence synergy\")\n\n")

(display ";; Convert AtomSpace patterns to prompts\n")
(display "(caichat-ggml-atomspace-to-prompt ggml-session ai-concept)\n\n")

(display "Example 5: Advanced Model Management\n")
(display "=====================================\n")
(display ";; Load a different model\n")
(display "(caichat-ggml-load-local-model \n")
(display "  \"/path/to/codellama-7b-instruct.ggmlv3.q4_0.bin\")\n\n")

(display ";; Get detailed model information\n")
(display "(caichat-ggml-model-info ggml-session)\n\n")

(display ";; Unload model when done\n")
(display "(caichat-ggml-unload-model ggml-session)\n\n")

(display "Example 6: Configuration Management\n")
(display "====================================\n")
(display ";; Auto-configure from environment variables\n")
(display "(caichat-auto-configure)\n\n")

(display ";; List available providers\n")
(display "(caichat-list-providers)\n\n")

(display ";; Get help\n")
(display "(caichat-help)\n\n")

(display "Example 7: Complete Workflow\n")
(display "=============================\n")
(display ";; Set up environment variable:\n")
(display ";; export GGML_MODEL_PATH=\"/path/to/model.ggmlv3.q4_0.bin\"\n\n")

(display ";; Complete workflow:\n")
(display "(use-modules (opencog caichat init))\n")
(display "(define session (caichat-ggml-session))\n")
(display "(define response (caichat-ggml-chat \"What is OpenCog?\"))\n")
(display "(display response)\n\n")

(display "╔══════════════════════════════════════════════════════════════╗\n")
(display "║                      Key Benefits                           ║\n")
(display "╚══════════════════════════════════════════════════════════════╝\n")
(display "✅ Local inference - no API keys or internet required\n")
(display "✅ Privacy-preserving - data never leaves your machine\n")
(display "✅ Cost-effective - no per-token charges\n")
(display "✅ Cognitive integration - seamless AtomSpace integration\n")
(display "✅ Extensible - supports multiple model formats\n")
(display "✅ Real-time - immediate responses without network latency\n\n")

(display "To use with real models:\n")
(display "1. Download GGML/GGUF format models (e.g., from Hugging Face)\n")
(display "2. Set GGML_MODEL_PATH environment variable\n")
(display "3. Install OpenCog and build CAIChat\n")
(display "4. Run the examples above!\n\n")

(display "For model downloads, check:\n")
(display "- https://huggingface.co/models?library=ggml\n")
(display "- https://huggingface.co/TheBloke (quantized models)\n\n")

(display "Happy coding with GGML + OpenCog! 🚀🧠\n")