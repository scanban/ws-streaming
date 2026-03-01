# Product Context

## Why this exists
Applications need low-latency, robust stream transport with rich signal metadata over WebSockets. This project provides a production-oriented C++ implementation that is easy to integrate into Asio-based systems.

## Problems solved
- Standardized streaming protocol implementation for C++ systems.
- Bidirectional signal transport over WebSocket.
- Metadata representation for typed/structured signals.
- Interop with legacy command-interface clients (JSON-RPC support on server side).

## How It Should Work

### Core Functionality
- **Client Role**: Connect to WebSocket servers, receive data streams, and process incoming signals
- **Server Role**: Accept connections from clients, manage multiple concurrent connections, and serve data streams
- **Signal Management**: Create, register, and manage data signals with rich metadata
- **Data Streaming**: Efficiently transmit scalar and structured data with minimal overhead

### Key Interactions
1. **Connection Management**: Automatic WebSocket handshake and protocol negotiation
2. **Signal Registration**: Register local signals for publishing and discover remote signals for consumption
3. **Data Flow**: Asynchronous data transmission with configurable buffering and flow control
4. **Error Handling**: Comprehensive error reporting and recovery mechanisms

## Primary users
- C++ developers building telemetry, control, or measurement systems.
- Systems already using Boost.Asio that need streaming protocol support.

## UX goals (developer experience)
1. Simple setup with CMake and minimal manual dependency management.
2. Clear client/server APIs with symmetric streaming concepts.
3. Practical examples for common patterns (server source, client sink, reverse direction, CAN data).

## Success Metrics
- **Performance**: minimum latency for data transmission
- **Reliability**: 99.9%+ connection uptime under normal conditions
- **Security**: maximum attention must be payed to the protocol safety
- **Developer Adoption**: Clear documentation and examples enabling rapid integration
- **Community**: Active contributions and issue resolution
