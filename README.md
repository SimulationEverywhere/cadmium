# cadmium [![Build Status](https://travis-ci.org/SimulationEverywhere/cadmium.svg?branch=master)](https://travis-ci.org/SimulationEverywhere/cadmium)

## Introduction
This library provides model classes for multiple Discrete-Event Simulation formalisms and the tools to simulate those models.
This project goal is replace CD++ with a more flexible and better performant implementation. Initial research in the  architecture of Cadmium can be found in [Sequential PDEVS architecture](http://cell-devs.sce.carleton.ca/publications/2015/VNWD15/) paper. Our primary building tool is clang, but we also test builds using gcc and visual studio when having the resources to do so.

## Top features
* Model validation at compile time.
* PDEVS models simulated in a single thread.
* Typed messages going through typed ports.
* Time representation is independent of model implementation.

## Quick start
###Requirements
* A C++14 compliant compiler. 

### Install
* The library is headers only. Then, it is enough to put the include directory in the path the compiler looks up for them.

### Building tests and examples
* Boost.Test, if running the testsfor running the tests.
* Boost.Build, if using the building files provided for convenience.

## References
* [CD++ website](http://cell-devs.sce.carleton.ca/mediawiki/index.php/Main_Page) is official CD++ website.
* [CD++ paper](http://www.sce.carleton.ca/faculty/wainer/papers/spe482.pdf) describes the CD++ simulator.
* [CDBoost website](http://blincubator.com/bi_library/simulation/?gform_post_id=1390) shows a preliminary implementation of Cadmium.
* [Sequential PDEVS architecture](http://cell-devs.sce.carleton.ca/publications/2015/VNWD15/) describes the architecture originally proposed. Several changes had been implemented since then. The documentation will explain them when ready.
