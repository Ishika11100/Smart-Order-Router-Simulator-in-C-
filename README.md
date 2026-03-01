# Smart Order Router Simulator in C++

Special Topic: Object-Oriented Programming I (22:839:614:40/41) – Spring 2026  
Instructor: Dr. Hseu-Ming Chen  
Group Memebrs: Ishika Patel, Emmanuel Ayinoluwa and George Marcu

## Project Overview

This project implements a **Smart Order Router (SOR) Simulator** in C++. The system models how a trading algorithm selects the most cost-efficient venue for executing orders when multiple exchanges are available, each with different spreads, fees, and liquidity characteristics.

The goal is to simulate execution quality and compare routing strategies in a simplified but realistic environment using object-oriented programming principles.


## Project Scope

The simulator includes:

- Three simulated trading venues
- Configurable bid-ask spread per venue
- Venue-specific transaction fee or rebate
- Simplified slippage model based on order size and liquidity
- Market order support
- Cost-based smart routing algorithm
- Execution performance metrics and reporting

This project does not attempt to replicate a full exchange infrastructure or high-frequency trading system.


## System Architecture

### Core Classes

- `Order` (base class)
- `MarketOrder` (derived class)
- `Venue`
- `SmartOrderRouter`
- `ExecutionResult`
- `Portfolio`
- `PerformanceAnalyzer`

### Design Principles Demonstrated

- Encapsulation
- Inheritance
- Polymorphism
- Modular separation of responsibilities
- Use of STL containers
- File input/output
- Exception handling


## Venue Simulation

Each venue contains:

- Bid-ask spread parameter
- Transaction fee or rebate
- Liquidity parameter affecting slippage

Execution price for a buy order:

Execution Price = Market Price + (Spread / 2) + Slippage

Slippage model:

Slippage = k × (Order Size / Liquidity)

This allows larger orders to incur higher execution cost while keeping implementation manageable.


## Smart Order Routing Logic

For each order, the Smart Order Router evaluates all available venues and calculates:

Expected Cost = Spread Cost + Fee Cost + Slippage Cost

The router selects the venue with the lowest expected execution cost.

For comparison, the system also implements a baseline routing strategy (e.g., always route to Venue 1).


## Execution Metrics

The simulator reports:

- Average execution price
- Total transaction fees
- Total slippage
- Fill rate
- Implementation shortfall

These metrics allow quantitative comparison between routing strategies.



## Development Phases

1. Core class design and architecture
2. Venue simulation and execution modeling
3. Smart routing logic implementation
4. Performance metrics and reporting
5. Integration, testing, and documentation


## Conclusion

The Smart Order Router Simulator demonstrates how object-oriented programming principles can be applied to a realistic financial execution problem. By modeling multiple trading venues and implementing a cost-based routing algorithm, the project highlights the importance of execution quality in trading systems while maintaining a clear and achievable scope.
