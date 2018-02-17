
			Some help for the betatesters trying the Pylon Addon beta 



0.91
---------------------------------------------------------
Some bugs corrected.. For now the release with rotation is simplified and the released object must be aligned with its parent, wich means that the child must have this DIR and ROT vectors in its first P attachment (this is temporary)
P 0 0 0 0 0 -1 0 1 0  XS

Other bugs related to boolean values were corrected.




0.9
---------------------------------------------------------




Well i suppose you already know what is this add-on for. I'll try to make a brief but complete explanation as a base for the future manual.. wich will be in pdf format of course :-) sorry about that.

PLEASE report any troubles/doubts/bugs/ideas to the Orbiter forum.

Thanks for trying this beta!!

Camelopardis




	1. REQUIREMENTS, LIMITATIONS...
---------------------------------------

For the pylon to work, the toparent attachment in the pylon must be defined with these dir and rot vectors: Dir = (0,0,-1), Rot = (0,1,0)

For example: 
P 3 4 5 0 0 -1 0 1 0 myAtt

Final not-pylon objects can have different Dir or Rot vectors than those.

A not-pylon vessel can't be attached (because a current Orbiter bug) to another not-pylon to form a tree with more than 2 levels. Said in another way, any not-pylon vessel can be in the tree, but it cannot have not-pylon childs except if it is the root of the tree. There's no checking for this. And won't be, in hope that the bug is corrected.

The classname of a pylon must start with "pylon". You can accomplish this by adding "pylon" to the classname, or also by simply putting the .cfg file in the \config\pylon subdirectory. You can't put the cfg in another subdirectory because this would make the classname different.

Example1:
The classname is pylonMyVesselClass (this is what you put in the scn file)
Config file is pylonMyVesselClass.cfg and is in \config directory.
The line 'classname = ' of the cfg file is "classname = pylonMyVesselClass"

Example2:
The classname is pylon\MyVesselClass
Config file is MyVesselClass.cfg and is in \config\pylon subdirectory.
The line 'classname = ' of the cfg file is "classname = MyVesselClass"

In both cases the 'module =' line in the cfg file must be "module = Pylon\Pylon"




	2. PYLON PARAMETERS
---------------------------

The pylon vessels 'publish' parameters that can be set within a sequence. The meaning and effect of setting or getting a parameter depends on the pylon module. Currently there is only one pylon module (Pylon.dll) but it publishes 21 parameters, that manage the rotation and traslation. Its parameter reference is at the end of this file.

There are 'user' parameters (they are visible in the Pylon MFD) and 'not-user' (only accesible from sequences) This is controlled by the module and you can do nothing about it.
The sequences also can be user or not-user, and there is a command for changing such state of a sequence.

There are four basic types for the parameters: boolean, integer, scalar and string. These four basic types are further divided, but that is only for readability. For example the boolean subtypes have different string representations (on/off, active/inactive..), and the scalar subtypes represent the magnitude (distance, velocity, acceleration...) but it has no other efect than displaying the magnitude in the MFD. The number of subtypes will grow in future versions without affecting anything. All these types can be used as parameters of the sequences (more on sequences below)

About units, they aren't displayed in the MFD. The base module defines its parameters in metric units (m, m/s, m/s/s) and angles are in degrees. But any other (future) module could define a parameter in whatever unit it wants, and that should be documented in its parameter list.

Here is the current list of types:

 Boolean types:
"Boolean"
	"Switch"
	"Activation"

 Integer types:
"Integer"

 Scalar types:
"Scalar"
	"Unitary"
	"Distance"
	"Velocity"
	"Acceleration"
	"Angle"
	"Angular velocity"
	"Angular acceleration"

 String types:
"String",
	"Object name"
	"Vessel name"
	"Vessel class"
	"GBody name"
	"Base name"
	"Parameter name"
	"Sequence name"
	"Pylon name"

The string subtypes are different. When used in a sequence command, they will be checked for validity. For example a vessel name must be an existing vessel name or the command will not be executed. The "Parameter name" and "Sequence name" subtypes have special syntax. They have two parts separated by a double period symbol. The first part is the pylon vessel name, and the second is the parameter name or the sequence name. The vessel name part is optional, and if it doesn't appear, it means that the parameter or sequence belongs to the pylon that is executing the command.

Example:
"a_Pylon:Rot_Angle" Refers to the Rot_Angle parameter of the a_Pylon vessel.
"a_Pylon:MySequence" Refers to the sequence MySequence in the same pylon vessel.
":Rot_Angle" Means the Rot_Angle parameter of the pylon wich is executing the sequence.

In the sequence commands, all values of type string must be enclosed by " ".
All values are case-sensitive, with some exceptions (Orbiter doesn't check the case with vessel names, gbody names, etc, but parameter and sequence names are case-sensitive)





	SEQUENCES
-----------------

The sequences are a limited script languaje. They doesn't support variables or arithmetic operations, that's the main limitation. But if you require variables and operations for what you want to do, you should start learning C... Anyways i'll be incrementing the number of commands, and i plan to do some high-level that will be very powerful. But let's continue with what we have in our hands..

Every sequence belongs to a Pylon. Currently the sequences are defined only in the .scn file, in the block of the pylon. But in a future version they will be loaded also from external .seq files, and each pylon class will have a matching .seq file that will be loaded on vessel creation.

There's a special sequence in the .scn file, "STATE", that is executed at the start of the simulation and is useful for setting the initial setup for the pylon. This sequence will also be saved on Orbiter exit or quicksave, to save the state of each pylon (again, in a future version)

A sequence is defined with the keyword BEGINSEQUENCE and ends with ENDSEQUENCE. Any line between them is supposed to be a sequence command. The sequences can NOT be nested! But they can call each other with the CALL_SEQ command.

The BEGINSEQUENCE line defines the name of the sequence, its parameter type if it has, its mode of execution, and wether it is a user sequence or not. 

A sequence can have one or no parameter. If it was defined a parameter type, a value must be passed to it when calling. If the sequence is called from the MFD, the value will be asked to the user, but if it is called by a CALL_SEQ command, the value will be given by the calling sequence.
The value passed as parameter to the sequence can be used by its commands.

Note that two sequences can't have the same name. If you define twice the same name, the previous sequence will be overwritten.


The sintax for defining a sequence is:

	BEGINSEQ "sequence name" [("parameter type")] [PARALLEL] [WAITONE] [USER]

The square brackets means an optional part (don't include them!). The parameter type is a string from the list above (don't forget the ""). The sequences have three modes of execution: sequential (default), parallel and 'waitone'.

In the sequential mode the sequence executes the commands from top to down, and until a command is not finished, the next command is not executed. Obviously the sequence finishes when its last command has been executed and finished. The only exceptions are that a invalid command is encountered or the sequence is cancelled, wich apply also for the two other modes.

The parallel mode is also intuitive: all the commands are executed in parallel, and the sequence doesn't finish until all of its commands have finished.

The last one is the WAITONE mode, wich is the same as parallel but the sequence finishes when just one command finishes, and the sequence doesn't care about the other commands. 

This modes, in conjunction with the commands WAIT_SEQUENCE, CANCEL_SEQUENCE and SET_CANCEL_SEQUENCE gives a good control of the execution.

Note that the STATE sequence is shot-execution: it won't wait for any command.

Examples of sequences:

This sequence is named Sequence1, has no parameter and the USER keywords says that it is visible and callable from the MFD:

	BEGINSEQUENCE "Sequence1" USER
	  commands...
	ENDSEQUENCE

This one is parallel, is not callable from the MFD and has a Scalar parameter.
	BEGINSEQUENCE "Sequence2" ("Scalar") PARALLEL
	  commands...
	ENDSEQUENCE

This one stops when any command finishes, is callable from the user and has a vessel name as parameter.
	BEGINSEQUENCE "Sequence3" ("Vessel name") WAITONE USER
	  commands...
	ENDSEQUENCE


Note that there can be spaces and tabs, and comments can be put with ';' as in the rest of Orbiter files.

When a pylon parameter is set (with the command SET_PARAM), the sequence executes it and then waits for it to terminate. But if the sequence is cancelled, the pylon will continue on its own until the command is finished. For example, if the "Ang_Rot" parameter of a pylon is set, the pylon will rotate to the given angle, wether or not the sequence has been cancelled or not. That's why the command SET_CANCEL_SEQUENCE exists.

Each sequence can have a cancel sequence, defined with the command SET_CANCEL_SEQUENCE. If a sequence is cancelled (by means of the MFD or the command CANCEL_SEQ, executed from another sequence) and hasn't been defined a cancel sequence, the cancelling is instantly done. But if it has a cancel sequence,  this one is called and the first sequence remains in the state 'Cancelling' (visible in the MFD) until the cancel sequence finishes.

Usually the cancel sequence should set the same parameters but to the initial state. For example, a sequence "UNFOLD_ANTENNA" could have as cancel sequence the sequence "FOLD_ANTENNA". Theres no need to set the first as the cancel sequence of the second, as calling the first one when it's cancelling effectively terminates the cancel sequence and start the normal execution of the first sequence.



	THE COMMANDS
--------------------


The format of a command line is simple:

	<command keyword> <value 1> <value 2> <value 3> ... <value N>

Each command has its list of parameters, wich are of a specific type. The list of current commands and its parameters and behaviour are below.

When a comand is executed, the value list is checked against the list of required parameters and its types. For example the command SET_MAIN_THRUST expects two parameters, the first a string containing the name of an existing vessel, and the second, the thust level (a "Scalar").

Nonetheless, all types are compatible. The boolean, integer and scalar types will be converted from one to another when necessary, and also the string ones. So for example it is the same to write
	SET_MAIN_THRUST "SH-1" 0.8 
as to write
	SET_MAIN_THRUST "SH-1" "0.8"

The boolean types are a bit special. They are converted to integer or scalar as the value 0 or 1, but will be converted to the strings "True" or "False" if the destination type is "String".
At the inverse, values are converted to a boolean "True" only if the value is non-zero (for integer or scalar types), and only if the string is not empty (for string values).

If the number of values passed is less than the required parameters, the command is invalidated and will make the sequence to abort. Nonetheless, there can be commands that have optional parameters (none of the current commands have them, though)


	SPECIAL COMMAND SYNTAX
------------------------------

There are a few special keywords that can be used instead of a constant (fixed) value as parameters of the commands. They are:

	1 - The sequence parameter symbol, < # >

By putting this symbol instead of a value, the sequence parameter will be passed. For example, in the "Sequence3" of the previous example, you could define the command:

SET_MAIN_THRUST # 0.5

This command would set the main thrust of the vessel passed as parameter to 0.5. It doesn't matter if the sequence parameter type was "String" or "Vessel name", the command would work the same. But in the second case, the sequence would check the validity of the name and wouldn't even start execution if the vessel name doesn't exist.

**kNOWN BUG: Making the final tests for the sample scenario i've discovered a bug: Passing a sequence parameter to another sequence as parameter doesn't work. Sorry, i'll try to fix this in the next beta.**


	2 - The 'get parameter' symbol, < ? >

This symbol gets the value of a pylon parameter (when the command is executed) and uses it as a parameter for the command. a value of type "Parameter name" must follow the symbol, with the usual syntax. Continuing with the same example, you could change the command to:

SET_MAIN_THRUST # ?"somevessel:someparameter"

Wich would read the parameter 'someparameter' of 'somevessel' and set it as the thrust for the (unknown) vessel received as parameter of the sequence.

I've planned more special symbols: to generate random numbers, to read some generic info of an object (position, velocity..), to reference the current vessel.. but for the moment there are only the two mentioned. The new symbols will start by an underscore (_) and followed by a keyword so you won't have to memorize a lot of symbols.



	LIST OF COMMANDS as of 0.9 beta version.
------------------------------------------------


In the descriptions i'll use the following format:

Command: <COMMAND_KEYWORD> <parameter1type> <parameter2type> <parameter3type> ...
Description: A description.
Duration: A command can be instantly executed or can involve an amount to time to complete.
Parameters: Description of the parameter values needed by the command.
Example: An example of use.

And the following abbreviations for parameter values:
	vessel: A vessel name.
	pylon: A Pylon vessel name.
	param: A Pylon vessel parameter name.
	seq: A Pylon vessel sequence name.
	value: A value of any type.
	<somename>(<someType>): a value of SomeType, with a name that helps understand the command.

Remember that all string types must be enclosed by "" and that the parameter names and sequence names use the syntax "pylonvesselname:param_name" or "pylonvesselname:sequence_name", and that the 'pylonvesselname' part is optional if the parameter or sequence belongs to the pylon executing the sequence command.

{ COMMAND LIST & DESCRIPTION HERE }


	Planned additional features
----------------------------------

* In version 1.0:
- CVelMessage support (commands for sending and waiting messages, accepting and executing commands from external sources...)





	Note about open source
------------------------------

The project will be released with sources under LGPL since version 1.0. Then other developers could interface with Pylon through code, and make additional Pylon modules with additional parameters that do specialized things.. who knows.





	Contributing
--------------------

Feel free to suggest ideas for the new versions to come (specially ideas about commands)






	APPENDIX: PARAMETER LIST FOR THE BASE PYLON MODULE
----------------------------------------------------------

A pylon - when attached to another vessel- can rotate on a principal axis (X, Y or Z) and have traslation defined with two arbitrary endpoints. The acceleration and maximum velocity can be set, as well as other parameters that affect its behaviour.

As said before, all units are metric and angles are in degrees.

"Rot_Angle" (Scalar) (User)
This in fact is the target or desired angle. The pylon will rotate until it is in the desired angle. This and the next are the only user parameters, all the rest must be set by sequences. Also, these two parameters are the only ones that can last some time to execute, all the others are finished instantly.

"Traslation" (Scalar) (User)
The same, but for the translation. This is the desired distance from the point (X0,Y0,Z0). The pylon will move in the line defined by (X0,Y0,Z0) and (X1,Y1,Z1)

"Rot_Axis" (String)
The accepted values are "X", "Y", "Z" and "" (empty string, wich means the pylon has no rotation).

"Has_Traslation" (Boolean)
Tells if the pylon has traslation movement. If not, the traslation parameters have no effect and the pylon stays in its initial position.

"Lin_Accel" (Scalar)
Acceleration for lineal movement. If the acceleration is 0, it means in fact infinity: the movement will be instantaneous (Curr_Trasl will be always equal to Traslation)

"Lin_Vel_Max" (Scalar)
Maximum traslation velocity.

"Ang_Accel" (Scalar)
The angular acceleration. As for traslation, if it is equal to 0, the movement is instantaneous. (Curr_Angle will be always equal to Rot_Angle)

"Ang_Vel_Max" (Scalar)
Maximum angular velocity.

"Ang_Min","Ang_Max" (Scalar)
Defines the limit angles for the rotation.

"X0", "Y0", "Z0", "X1", "Y1", "Z1" (Scalar)
These are the two points that define the traslation, in local vessel coordinates.

"Clamp" (Boolean)
If set to 1, the limit angles will take effect, and the traslation will not pass the endpoints. If set to 0, the rotation can be continuous, and the traslation can pass over the endpoints without any limit.

"Curr_Trasl" (Scalar)
This is the current traslation.

"Lin_Vel" (Scalar)
The current velocity.

"Curr_Angle" (Scalar)
The current angle of rotation.

"Ang_Vel" (Scalar)
And the last but not less important, the current angular velocity.

Note: Changing some not-user parameters in mid-simulation could give a unrealistic effect. For example changing the rotation axis while the pylon is rotating, because the rotation is always referenced to the initial orientation. Changing the current angle will move instantly the vessel, also will do the current traslation, etc.


