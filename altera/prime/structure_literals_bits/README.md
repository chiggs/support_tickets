To build with Quartus run `make quartus`, to build with Quartus Prime run `make prime`


Building this testcase under Quartus 15.1 original synthesis engine the result correctly reports:

```                                                                                                                            
   Info (21058): Implemented 1 input pins                                                                                                                                                                                                   
   Info (21059): Implemented 33 output pins                                                                                                                                                                                                 
   Info (21061): Implemented 34 logic cells
```

However under using Spectra-Q synthesis, the structure literal seems to be optimised to nothing, giving the following synthesis result:

```
   Info (21058): Implemented 1 input pins
   Info (21059): Implemented 33 output pins
   Info (21061): Implemented 4 logic cells
```

In this case, the old Quartus engine is correct and SpectraQ is wrong.

