.. _SignalTypes:

Signal Types
============

TiA defines the following signals types.

==================== ============= ============ ========================
Identifier (string)  Value (hex)   Value (dec)  Description
==================== ============= ============ ========================
``eeg``              0x00000001    1            Electroencephalogram
``emg``              0x00000002    2            Electromyogram    
``eog``              0x00000004    4            Electrooculogram
``ecg``              0x00000008    8            Electrocardiogram  
``hr``               0x00000010    16           Heart rate
``bp``               0x00000020    32           Blood pressure  
``button``           0x00000040    64           Buttons (aperiodic)
``joystick``         0x00000080    128          Joystick axis (aperiodic)
``sensors``          0x00000100    256          Sensor
``nirs``             0x00000200    512          NIRS
``fmri``             0x00000400    1,024        FMRI
``mouse``            0x00000800    2,048        Mouse axis (aperiodic)
``mouse-button``     0x00001000    4,096        Mouse buttons (aperiodic)
not used yet
``user_1``           0x00010000    65,536       User 1
``user_2``           0x00020000    131,072      User 2
``user_3``           0x00040000    262,144      User 3
``user_4``           0x00080000    524,288      User 4
``undefined``        0x00100000    1,048,576    undefined signal type
``event``            0x00200000    2,097,152    event
==================== ============= ============ ========================


Aperiodic Signals
*****************

Aperiodic signals are signals which are not transmitted at constant time rates. For example the state of joystick
axis.

Therefore values of the current state of these signals have be transmitted only if they have changed.