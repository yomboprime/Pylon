#include "stdio.h"
#include "PylonSequence.h"
#include "CPylon.h"

// ==========================================================
// Pylon Sequence Class
// ==========================================================

PylonSequence::PylonSequence(bool mainSeq, void *owner) {
	mainSequence = mainSeq;
	valid = true;
	mode = SEQ_MODE_SEQUENTIAL;
	userSequence = false;
	activate = false;
	cancel = false;
	activating = false;
	cancelling = false;
	finished = true;
	finishedcommand = false;

	activationKey = 0;


	this->owner = owner;

	if (mainSequence) {
		caller = NULL;
		cmdcurrent = NULL;
		name = new char[NAME_SIZE];
	} else {
		caller = NULL;
		cmdcode = PYL_CMD_NOT_DEFINED;
		name = NULL;
	}

	value.type = PYL_PARAM_NOT_DEFINED;
	value.tempRef = NULL;
	value.tempIndex = -1;

	nextCmd = NULL;
	nextSeq = NULL;
}

PylonSequence::~PylonSequence() {
// todo: DOH! this destructor has nothing to do with other sequences so don't delete
// nextSeq unless this is a sequence cmd, where nextSeq contains the parameters. So the
// mainSequence flag must be checked for this. Copy the code of CPylon::DeleteSequence, the loop
// where parameters are deleted (in case where a parameter is PYL_PARAM_GET_PARAMETER, its string value
// must be deleted also)
// Here also the name can be deleted
// Don't touch nextSeq/nextCmd pointers
// In CPylon::DeleteSequence only is deleted the sequence and the chain is 'rechained', nothing more will be done.

//	if (name) delete name;
//	if (nextSeq) delete nextSeq;
//	if (nextCmd) delete nextCmd;
}

void PylonSequence::Print(char *str, int strSize) {
	if (mainSequence) {
		int n = _snprintf_s(str, NAME_SIZE, NAME_SIZE, "BEGINSEQ \"%s\"",name);
		if (strSize < n) { *str = 0; return; }
		str+= n; strSize-=n;
		if (value.type==PYL_PARAM_NOT_DEFINED) {
			if (strSize < 2) { *str = 0; return; }
		} else {
			n=strlen(PylonParamTypeName[value.type]);
			if (strSize < n + 2) { *str = 0; return; }
			strSize-=n+2;
			*str=' ';str++;
			*str='(';str++;
			_snprintf_s(str, NAME_SIZE, NAME_SIZE, "\"%s\"",PylonParamTypeName[value.type]);
			str+= n + 2;
			*str=')';str++;
		}
		n=0;
		if (strSize < 5) { *str = 0; return; }
		if (userSequence) n = _snprintf_s(str, NAME_SIZE, NAME_SIZE, " USER"); else n=0;
		str+= n; strSize-=n;
		if (strSize < 9) { *str = 0; return; }
		if (mode == SEQ_MODE_PARALLEL) n = _snprintf_s(str, NAME_SIZE, NAME_SIZE, " PARALLEL");
		else if (mode == SEQ_MODE_WAIT_ONE) n = _snprintf_s(str, NAME_SIZE, NAME_SIZE, " WAIT_ONE");
		str+= n; strSize-=n;
		*str = 0;

	} else {
		if (cmdcode != PYL_CMD_NOT_DEFINED) {
			strSize--;
			int n = _snprintf_s(str, NAME_SIZE, NAME_SIZE, "%s",PylonCommands[cmdcode].name);
			str+= n; strSize-=n;
		}
		if (cmdcode == PYL_CMD_NOT_DEFINED || PylonCommands[cmdcode].numParams > 0) {
			static char s[NAME_SIZE];

			if (value.type == PYL_PARAM_GET_PARAMETER)
				_snprintf_s(s, NAME_SIZE, NAME_SIZE, "?\"%s\"",name);
			else if (value.type == PYL_PARAM_GENERATE_UNIFORM)
				_snprintf_s(s, NAME_SIZE, NAME_SIZE, "_U(%f,%f)",value.doubleArray[0],value.doubleArray[1]);
			else if (value.type == PYL_PARAM_GENERATE_NORMAL)
				_snprintf_s(s, NAME_SIZE, NAME_SIZE, "_N(%f,%f)",value.doubleArray[0],value.doubleArray[1]);
			else if (value.type == PYL_PARAM_THIS)
				_snprintf_s(s, NAME_SIZE, NAME_SIZE, "_THIS");

			else if (value.type < PYL_PARAM_INTEGER) _snprintf_s(s, NAME_SIZE, NAME_SIZE, "%d", value.bolValue?1:0);
			else if (value.type < PYL_PARAM_SCALAR) _snprintf_s(s, NAME_SIZE, NAME_SIZE, "%d", value.intValue);
			else if (value.type < PYL_PARAM_STRING) _snprintf_s(s, NAME_SIZE, NAME_SIZE, "%f", value.dblValue);
			//else if (value.type == PYL_PARAM_PARAMETER_NAME) _snprintf_s(s, NAME_SIZE, NAME_SIZE, "\"%s:%s\"",((CPylon*)(value.tempRef))->GetName(),value.strValue);
			//else if (value.type == PYL_PARAM_PARAMETER_NAME) _snprintf_s(s, NAME_SIZE, NAME_SIZE, "\">OK>%s<<[%d]\"",value.strValue,value.tempRef);
			//else if (value.type < PYL_PARAM_NOT_DEFINED) _snprintf_s(s, NAME_SIZE, NAME_SIZE, "\">>%s<<[%d]\"",value.strValue,value.tempRef);
			else if (value.type < PYL_PARAM_NOT_DEFINED) _snprintf_s(s, NAME_SIZE, NAME_SIZE, "\"%s\"",value.strValue);
			else _snprintf_s(s, NAME_SIZE, NAME_SIZE, "#");
			int n = strlen(s);
			if (strSize < n+1) { *str = 0; return; }
			_snprintf_s(str, NAME_SIZE, NAME_SIZE, " %s",s);
			str+= n+1; strSize-=n+1;
		}
		if (nextCmd==NULL || strSize==0) { *str = 0; return; }
		*str= ' ';str++;strSize--;
		nextCmd->Print(str, strSize);
	}
}

char *PylonSequence::AddCmd(char *cmd, TPylParamValue &val) {
	if (!mainSequence) return NULL;

	char *lastpos = GetToken(cmd, val);
	if (lastpos==NULL) return NULL;
	if (val.type != PYL_PARAM_TOKEN_COMMAND) return cmd;

	PylonSequence *command = nextCmd;
	if (command==NULL) {
		nextCmd = new PylonSequence(false, owner);
		command = nextCmd;
	} else {
		while (command->nextSeq!=NULL) command = command->nextSeq;
		command->nextSeq = new PylonSequence(false, owner);
		command = command->nextSeq;
	}
	command->cmdcode = val.intValue;
	command->caller = this;

	PylonSequence *param = command;
	int n = PylonCommands[command->cmdcode].numParams;
	for (int i=0;i<n;i++) {
		if (i>0) {
			param->nextCmd = new PylonSequence(false, owner);
			param = param->nextCmd;
		}
		param->caller = this;
		lastpos = GetToken(lastpos, val);
		if (lastpos==NULL) { param->valid = false; return NULL; }
		if (val.type==PYL_PARAM_GET_PARAMETER) {
			param->value.type = val.type;
			lastpos = GetToken(lastpos, val);
			if (lastpos==NULL || val.type != PYL_PARAM_STRING) { param->valid = false; return NULL; }
			param->name = new char[strlen(val.strValue)+1];
			_snprintf_s(param->name, NAME_SIZE, NAME_SIZE, "%s",val.strValue);
		} else if (val.type==PYL_PARAM_GENERATE_UNIFORM || val.type==PYL_PARAM_GENERATE_NORMAL) {
			param->value.type = val.type;
			lastpos = GetToken(lastpos, val);
			if (lastpos==NULL || val.type!=PYL_PARAM_TOKEN_OPENBR) { param->valid = false; return NULL; }
			lastpos = GetToken(lastpos, val);
			if (lastpos==NULL || val.type != PYL_PARAM_SCALAR) { param->valid = false; return NULL; }
			param->value.doubleArray[0] = val.dblValue;
			lastpos = GetToken(lastpos, val);
			if (lastpos==NULL || val.type!=PYL_PARAM_TOKEN_COMMA) { param->valid = false; return NULL; }
			lastpos = GetToken(lastpos, val);
			if (lastpos==NULL || val.type != PYL_PARAM_SCALAR) { param->valid = false; return NULL; }
			param->value.doubleArray[1] = val.dblValue;
			lastpos = GetToken(lastpos, val);
			if (lastpos==NULL || val.type!=PYL_PARAM_TOKEN_CLOSEBR) { param->valid = false; return NULL; }
		} else if (val.type==PYL_PARAM_THIS) {
			param->value.type = val.type;
		} else if (val.type==PYL_PARAM_NOT_DEFINED) {
			param->value.type = val.type;
		} else {
			param->value.type = PylonCommands[command->cmdcode].paramType[i];
			if (param->value.type==PYL_PARAM_NOT_DEFINED) param->value.type = val.type;
			param->ConvertValue(param->value.type, val.type, &val, false);
		}
	}
	if ( strcmp(name,"STATE")!=0 )
	{
		if (command->cmdcode == PYL_CMD_SET_CANCEL_SEQUENCE ) {
/*char s[80];
command->Print(s,80);
_snprintf_s(oapiDebugString(), NAME_SIZE, NAME_SIZE, "createSETCANCELSEQ: (%f)",oapiGetSimTime());
*/
			command->activate = true;
			command->Execute();
		}
	}

	return NULL;
}
void PylonSequence::SetCancelSequence(PylonSequence *ps) {

	if (!mainSequence) return;
/*_snprintf_s(oapiDebugString(), NAME_SIZE, NAME_SIZE,
"setcancelseq()1: this=%s, canseq=%s",//, type=%d, strvalue=%s, tempidx=%d, tempref=%d, canseq=%s",
name,ps->name);//,value.type, value.strValue, value.tempIndex, value.tempRef, ps?ps->name:"NOPS");
*/
	if (nextCmd==NULL) {
		nextCmd = new PylonSequence(false, owner);
		nextCmd->cmdcode = PYL_CMD_SET_CANCEL_SEQUENCE;
		nextCmd->value.tempRef = ps;
	} else if (nextCmd->cmdcode==PYL_CMD_SET_CANCEL_SEQUENCE) {
		nextCmd->value.tempRef = ps;
	} else {
		PylonSequence *p = new PylonSequence(false, owner);
		p->cmdcode = PYL_CMD_SET_CANCEL_SEQUENCE;
		p->value.tempRef = ps;
		p->nextSeq = nextCmd;
		nextCmd = p;
	}

}
bool PylonSequence::Call(TPylParamValue *val, PylonSequence *caller) {
/*char s[NAME_SIZE];
Print(s,NAME_SIZE);
_snprintf_s(oapiDebugString(), NAME_SIZE, NAME_SIZE, "CALL19: <%s> valt=%d,valstrvalue=%s",s,val->type,val->strValue);
*/
	if (!mainSequence || !valid || activating || activate) return false;
	if (value.type!=PYL_PARAM_NOT_DEFINED && val->type==PYL_PARAM_NOT_DEFINED) return false;

	if (nextCmd==NULL) return false;
	if (nextCmd->cmdcode == PYL_CMD_SET_CANCEL_SEQUENCE && nextCmd->nextSeq==NULL ) return false;

//bool b = CastValue(0, val);
//_snprintf_s(oapiDebugString(), NAME_SIZE, NAME_SIZE, "CALL22: <%s> valt=%d,valstrvalue=%s,casted:%d",s,val->type,val->strValue,b?1:0);
	if (!CastValue(0, val)) return false;

//_snprintf_s(oapiDebugString(), NAME_SIZE, NAME_SIZE, "CALL20: <%s>"/*-> %d,%d,%d"*/,s);//,nextCmd->cmdcode,nextCmd->nextSeq,nextCmd->nextCmd);

	this->caller = caller;
	activate = true;
	activating = true;
	finished = false;
	return true;
}
bool PylonSequence::Cancel(TPylParamValue *val) {
	if (!mainSequence || !valid || cancelling || cancel) return false;
//_snprintf_s(oapiDebugString(), NAME_SIZE, NAME_SIZE, "CANCEL10: valid=%d, nextcmdcode=%s,cancelseq=%s",nextCmd?(nextCmd->valid?1:0):-1, nextCmd?PylonCommands[nextCmd->cmdcode].name:"NOCMD", nextCmd->value.tempRef?((PylonSequence*)nextCmd->value.tempRef)->name:"NOTEMPREF");

	this->caller = NULL;
	cancel= true;
	finished = false;
	cancelling = true;


	PylonSequence *p = nextCmd;
	if (p!=NULL && p->cmdcode==PYL_CMD_SET_CANCEL_SEQUENCE && p->valid) {

		if (((PylonSequence*)(p->value.tempRef))->value.type!=PYL_PARAM_NOT_DEFINED && val->type==PYL_PARAM_NOT_DEFINED) {
			return false;
		}
/*_snprintf_s(oapiDebugString(), NAME_SIZE, NAME_SIZE, "CANCEL12: cancelseq=%s,cancelseqtype=%d,valtype=%d",
		p->value.tempRef?((PylonSequence*)p->value.tempRef)->name:"NOTEMPREF",
		((PylonSequence*)(p->value.tempRef))->value.type,
		val->type);
*/
		((PylonSequence*)(p->value.tempRef))->Call(val, this);
	}
	return true;
}
bool PylonSequence::Execute(void) {
//char s[NAME_SIZE];
	if (mainSequence) {
		if (finished) {
			if (nextSeq) nextSeq->Execute();
			return true;
		}

//Print(s,NAME_SIZE);
//_snprintf_s(oapiDebugString(),NAME_SIZE, NAME_SIZE, "Exec mainseq2: %s, mode=%d,activ=%d,type=%d,strvalue=%s",s,mode,activate?1:0,value.type,value.strValue);

			if (activate) {

				if (nextCmd==NULL || (nextCmd->cmdcode==PYL_CMD_SET_CANCEL_SEQUENCE &&
					nextCmd->nextSeq==NULL))
					return true;

				if (cancelling && nextCmd && nextCmd->cmdcode==PYL_CMD_SET_CANCEL_SEQUENCE) {
					nextCmd->Terminate();
				}
				activate = false; activating = true; cancelling = false;
				finishedcommand = false;

				if (mode > SEQ_MODE_SEQUENTIAL) {
//_snprintf_s(oapiDebugString(), NAME_SIZE, NAME_SIZE, "%s: PARALLEL Activate: (%f)",name,oapiGetSimTime());
					PylonSequence *ps = nextCmd;
					if (ps && ps->cmdcode == PYL_CMD_SET_CANCEL_SEQUENCE)
						ps = ps->nextSeq;
					bool term = (mode == SEQ_MODE_PARALLEL);
					while (ps) {
						if (ps->valid) {
							ps->activate = true;
							ps->finished = false;
							ps->finishedcommand = false;
							if (!ps->Execute()) {
								if (mode==SEQ_MODE_PARALLEL) term = false;
							} else {
								ps->finishedcommand=true;
								if (mode==SEQ_MODE_WAIT_ONE) term = true;
							}
						}
						else {
							//error de comando no valido ejecutado en paralelo...
							ps->finishedcommand=true;
							if (mode==SEQ_MODE_WAIT_ONE) term = true;
						}
						ps = ps->nextSeq;
					}
					if (term) {
//_snprintf_s(oapiDebugString(), NAME_SIZE, NAME_SIZE, "Ejec1: %s term (%f)",name,oapiGetSimTime());
						finishedcommand = true;
						finished = true;
						activating = false;
						cancelling = false;
					}
				} else {
					cmdcurrent = nextCmd;
					if (cmdcurrent && cmdcurrent->cmdcode == PYL_CMD_SET_CANCEL_SEQUENCE)
						cmdcurrent = cmdcurrent->nextSeq;
					while (cmdcurrent) {
						if (cmdcurrent->valid) {
							cmdcurrent->finished = false;
							cmdcurrent->activate = true;
							if (cmdcurrent->Execute()) {
								cmdcurrent = cmdcurrent->nextSeq;
							}
							else break;
						} else cmdcurrent = NULL;
					}
					if (cmdcurrent == NULL) {
						finished = true;
						finishedcommand = true;
						activating = false;
						cancelling = false;
					}
				}
			}

			if (cancel) {
//3
//_snprintf_s(oapiDebugString(), NAME_SIZE, NAME_SIZE, "%s: Cancel1 (%f)",name,oapiGetSimTime());
				Terminate();
				finished = false;
				cancelling = true;
			}

			if (activating) {
				if (mode > SEQ_MODE_SEQUENTIAL) {
//_snprintf_s(oapiDebugString(), NAME_SIZE, NAME_SIZE, "%s: PARALLEL ActivatING: (%f)",name,oapiGetSimTime());
					PylonSequence *ps = nextCmd;
					if (ps && ps->cmdcode == PYL_CMD_SET_CANCEL_SEQUENCE)
						ps = ps->nextSeq;
					bool term = (mode == SEQ_MODE_PARALLEL);
					while (ps) {
						if (!ps->finishedcommand) {
							if (ps->valid) {
								if (!ps->Execute()) {
									if (mode==SEQ_MODE_PARALLEL) term = false;
								} else {
									ps->finishedcommand=true;
									if (mode==SEQ_MODE_WAIT_ONE) term = true;
								}
							}
							else {
								//error de comando no valido ejecutado en paralelo...
								ps->finishedcommand=true;
								if (mode==SEQ_MODE_WAIT_ONE) term = true;
							}
						}
						ps = ps->nextSeq;
					}
					if (term) {
						finishedcommand = true;
						finished = true;
						activating = false;
						cancelling = false;
					}
				}
				else {
					if (cmdcurrent && cmdcurrent->cmdcode == PYL_CMD_SET_CANCEL_SEQUENCE)
						cmdcurrent = cmdcurrent->nextSeq;

					while (cmdcurrent) {
						if (cmdcurrent->valid) {
							if (cmdcurrent->Execute()) {
//								_snprintf_s(oapiDebugString(), NAME_SIZE, NAME_SIZE, "param ejecutado: cmd=%d, strValue=%s",cmdcurrent->cmdcode, cmdcurrent->nextCmd?cmdcurrent->nextCmd->value.strValue:"no cmd");
								cmdcurrent = cmdcurrent->nextSeq;
								if (cmdcurrent) {
									cmdcurrent->finished = false;
									cmdcurrent->activate = true;
								}
							}
							else break;
						} else cmdcurrent = NULL;
					}
					if (cmdcurrent == NULL) {
						finished = true;
						finishedcommand = true;
						activating = false;
						cancelling = false;
					}
				}

			} else if (cancelling) {
//_snprintf_s(oapiDebugString(), NAME_SIZE, NAME_SIZE, "%s: Cancelling (%f)",name,oapiGetSimTime());
				PylonSequence *p = nextCmd;
				if (p && p->cmdcode==PYL_CMD_SET_CANCEL_SEQUENCE && p->valid) {
					if (((PylonSequence*)p->value.tempRef)->finished) {
						finished = true;
						cancelling = false;
					}
				} else {
					finished = true;
					cancelling = false;
				}
			}
			if (nextSeq) nextSeq->Execute();
			return finishedcommand;


		} else {

			// Command execution
			if (cmdcode  < 0 || cmdcode >= PYL_CMD_NOT_DEFINED) valid = false;
			if (!valid) return true;

			VESSEL *ves, *ves2, *ves3;
			CPylon *p;
			char strTemp[NAME_SIZE];
			char *c;
			PylonSequence *ps;
			TPylParamValue *v;
			int paramt, i, n;
			DWORD dw, dw2;
			PROPELLANT_HANDLE propH;
			THRUSTER_HANDLE thrustH;
			VECTOR3 vector[6];
			double d, d2, d3;
			MATRIX3 matrix;
			PylonMesh *pmesh;

//Print(strTemp,NAME_SIZE);
//_snprintf_s(oapiDebugString(), NAME_SIZE, NAME_SIZE, "Ejec cmd: %s>%s (%f)",caller->name,strTemp,oapiGetSimTime());

			if (activate) {
				int np = PylonCommands[cmdcode].numParams; i=0;
				ps = this;
				while (ps && i<np) {

					if (!ps->CastValue(PylonCommands[cmdcode].paramType[i], NULL)) {
						return true;
					}

					ps = ps->nextCmd; i++;
				}
				if (i<np)
					return true;
			}


			OBJHANDLE objH, obj2H;
			ATTACHMENTHANDLE attH, attH2;
			VESSELSTATUS2 status;

			bool lShift, rShift, lCtrl, rCtrl, lAlt, rAlt, modifiers;

//   param: "nam" or "obj:nam"
//   seq: "nam" or "obj:nam"
//   value: cast the value to the param or seq type.
/*				 Values:
					.eE+-0..9 : double (also integer, boolean)
					"" : string
					_ prefix of special keyword
					# : get the sequence parameter
					?nam or ?obj:nam : read the parameter
*/

			switch (cmdcode) {

				/*
					SET_PARAM param value
				*/
				case PYL_CMD_SET_PARAM:

					p = (CPylon*)(value.tempRef);
					if (activate) {
						activate = false;
						paramt = p->GetParameterType(value.tempIndex);
						v = &nextCmd->value;
						nextCmd->ConvertValue(paramt, v->type,v,true);
						return p->SetParam(value.tempIndex, v);
					} else {
						return p->IsParameterSet(value.tempIndex);
					}
					break;

				/*
					CALL_SEQ sequence value
				*/
				case PYL_CMD_CALL_SEQ:
					p = (CPylon*)(value.tempRef);
					if (activate) {
						activate = false;
						if (nextCmd==NULL) {
							return !p->ActivateSequence(value.tempIndex);
						} else {
							TPylParamValue *v = &nextCmd->value;
							if (!nextCmd->CastValue(p->GetSequenceType(value.tempIndex), NULL))
								return true;
							return !p->ActivateSequenceValue(value.tempIndex, v);
						}
					} else {
//_snprintf_s(oapiDebugString(), NAME_SIZE, NAME_SIZE, "EjecCALL: %s:%d, active=%d (%f)",p->GetName(),value.tempIndex,p->IsSequenceActive(value.tempIndex)?1:0,oapiGetSimTime());
						return !p->IsSequenceActive(value.tempIndex);
					}
					return true;
					break;

				/*
					CANCEL_SEQ sequence value
				*/
				case PYL_CMD_CANCEL_SEQ: // seq value
					p = (CPylon*)(value.tempRef);
					if (p==NULL) return true;
					if (activate) {
						activate = false;
						TPylParamValue *v = &nextCmd->value;
						if (v==NULL) {
							if (!p->CancelSequence(value.tempIndex)) return true;
						} else
							if (!!p->CancelSequenceValue(value.tempIndex, v)) return true;
					}
					return !p->IsSequenceCancelling(value.tempIndex);
					break;

				/*
					SET_CANCEL_SEQUENCE sequence
				*/
				case PYL_CMD_SET_CANCEL_SEQUENCE: //seq
					p = (CPylon*)(value.tempRef);
					n = p->GetSequenceCount();
					if (n<=value.tempIndex) return true;
					n = value.tempIndex;
					ps = p->sequences;
					while (n>0) {ps=ps->nextSeq;n--;}

					caller->SetCancelSequence(ps);
					return true;
					break;

				/*
					PYL_CMD_SET_SEQUENCE_KEY sequence key
				*/
				case PYL_CMD_SET_SEQUENCE_KEY: //key

				    p = (CPylon*)(value.tempRef);
					if (p==NULL) return true;
					dw = value.tempIndex;
					ps = p->sequences;
					while (dw>0 && ps) {ps=ps->nextSeq;dw--;}
					if (ps==NULL) return true;
                    ps->activationKey = nextCmd->value.intValue;
					return true;
					break;

				// Logic control
				case PYL_CMD_RESET:
					if (!this->caller) return true;
					ps = ((PylonSequence *)this->caller);
					ps->activate = true;
					ps->activating = true;
					ps->finished = false;
					return false;
					break;

				case PYL_CMD_SKIP_IF: return true; //bool
				case PYL_CMD_SKIP_IF_NOT: return true; //bool
				case PYL_CMD_RESET_IF: return true; //bool
				case PYL_CMD_RESET_IF_NOT: return true; //boo

				// Time events

				/*
					DELAY seconds
				*/
				case PYL_CMD_DELAY:
					if (activate) {
						activate = false;
						if ( oapiGetSimStep() > value.dblValue ) return true;
						if (value.dblValue<=0) return true;
						value.doubleArray[0] = oapiTime2MJD(oapiGetSimTime() + value.dblValue);
						return false;
					} else {
						return value.doubleArray[0] <= oapiGetSimMJD();
					}
					break;

				/*
					DELAY_UNTIL_MJD seconds
				*/
				case PYL_CMD_DELAY_UNTIL_MJD: return true;
					if (activate) {
						activate = false;
						if (value.dblValue <= oapiGetSimMJD()) return true;
						return value.dblValue <= oapiGetSimMJD();
					} else {
						return value.dblValue <= oapiGetSimMJD();
					}
					break;

				/*
					DELAY_UNTIL_SIMT seconds
				*/
				case PYL_CMD_DELAY_UNTIL_SIMT: return true; //seconds(scalar)
					if (activate) {
						activate = false;
						if (value.dblValue<=0) return true;
						value.doubleArray[0] = oapiTime2MJD(value.dblValue);
					}
					return value.doubleArray[0] <= oapiGetSimTime();

				// Other events

				/*
					WAIT_SEQUENCE sequence
				*/
				case PYL_CMD_WAIT_SEQUENCE:
					activate = false;
					p = (CPylon*)(value.tempRef);
					if (p==NULL) return true;
					return !p->IsSequenceActive(value.tempIndex);

				/*
					WAIT_FUEL_LEVEL vessel propellant_tank level
				*/
				case PYL_CMD_WAIT_FUEL_LEVEL:
					activate = false;
					ves = (VESSEL*)(value.tempRef);
					dw = nextCmd->value.intValue;
					if (dw >= ves->GetPropellantCount()) return true;
					propH = ves->GetPropellantHandleByIndex(dw);
					d = ves->GetPropellantMaxMass(propH);
					if (d == 0) return true;
					return (ves->GetPropellantMass(propH) / d) <= nextCmd->nextCmd->value.dblValue;

				/*
					WAIT_THRUSTER_LEVEL vessel thruster level
				*/
				case PYL_CMD_WAIT_THRUSTER_LEVEL:
					activate = false;
					ves = (VESSEL*)(value.tempRef);
					dw = nextCmd->value.intValue;
					if (dw >= ves->GetThrusterCount()) return true;
					thrustH = ves->GetThrusterHandleByIndex(dw);
					d = ves->GetThrusterLevel(thrustH);
					return d >= nextCmd->nextCmd->value.dblValue;

				// Setting vessel status
				/*
					ATTACH vessel(parent) int(tochild) int(toparent) vessel(child)
				*/
				case PYL_CMD_ATTACH:
					ves = (VESSEL*)value.tempRef;
					dw = nextCmd->value.intValue;
					dw2 = nextCmd->nextCmd->value.intValue;
					ves2 = (VESSEL*)(nextCmd->nextCmd->nextCmd->value.tempRef);
					if (dw < 0 || dw >= ves->AttachmentCount(false) ||
						dw2 < 0 || dw2 >= ves2->AttachmentCount(true)) return true;
					CPylon::PylonAttach(ves->GetHandle(),
										ves2->GetHandle(),
										ves->GetAttachmentHandle(false,dw),
										ves2->GetAttachmentHandle(true,dw2));
					return true;

				/*
					ATTACH_DISTANCE vessel(parent) int(tochild) int(toparent) vessel(child) scalar(distance)
				*/
				case PYL_CMD_ATTACH_DISTANCE:
					dw = nextCmd->value.intValue;
					dw2 = nextCmd->nextCmd->value.intValue;
					ves = (VESSEL*)value.tempRef;
					ves2 = (VESSEL*)(nextCmd->nextCmd->nextCmd->value.tempRef);

					ves2->GetAttachmentParams(ves2->GetAttachmentHandle(true, dw2), vector[0], vector[1], vector[2]);
					ves2->Local2Global(vector[0], vector[3]);
					ves->GetAttachmentParams(ves->GetAttachmentHandle(false, dw), vector[0], vector[1], vector[2]);
					ves->Local2Global(vector[0], vector[4]);
					vector[5] = vector[3] - vector[4];

					d = nextCmd->nextCmd->nextCmd->nextCmd->value.dblValue;
					vector[0].x = length(vector[5]);
					if ( length(vector[5]) <= d )
					{
						if (dw < 0 || dw >= ves->AttachmentCount(false) ||
							dw2 < 0 || dw2 >= ves2->AttachmentCount(true)) return true;
						CPylon::PylonAttach(ves->GetHandle(),
											ves2->GetHandle(),
											ves->GetAttachmentHandle(false,dw),
											ves2->GetAttachmentHandle(true,dw2));
						return true;
					}
					return false;

				/*
					ATTACH_ALIGNED vessel(parent) int(tochild) int(toparent) vessel(child) scalar(distance) scalar(angle)
				*/
				case PYL_CMD_ATTACH_ALIGNED:
					dw = nextCmd->value.intValue;
					dw2 = nextCmd->nextCmd->value.intValue;
					ves = (VESSEL*)value.tempRef;
					ves2 = (VESSEL*)(nextCmd->nextCmd->nextCmd->value.tempRef);

					ves2->GetAttachmentParams(ves2->GetAttachmentHandle(true, dw2), vector[0], vector[1], vector[2]);
					ves2->Local2Global(vector[0], vector[3]);
					ves->GetAttachmentParams(ves->GetAttachmentHandle(false, dw), vector[0], vector[1], vector[2]);
					ves->Local2Global(vector[0], vector[4]);
					vector[5] = vector[3] - vector[4];

					d = nextCmd->nextCmd->nextCmd->nextCmd->value.dblValue;
					d2 = length(vector[5]);
					if ( d2 <= d )
					{
						d = cos( RAD * nextCmd->nextCmd->nextCmd->nextCmd->nextCmd->value.dblValue );

						ves2->GetAttachmentParams(ves2->GetAttachmentHandle(true, dw2), vector[0], vector[1], vector[2]);
						ves2->GlobalRot(vector[1], vector[3]);
						ves->GetAttachmentParams(ves->GetAttachmentHandle(false, dw), vector[0], vector[1], vector[2]);
						ves->GlobalRot(vector[1], vector[4]);
						d2 = dotp( vector[3], vector[4] );
						if (-d2 < d ) return false;

						ves2->GetAttachmentParams(ves2->GetAttachmentHandle(true, dw2), vector[0], vector[1], vector[2]);
						ves2->GlobalRot(vector[2], vector[3]);
						ves->GetAttachmentParams(ves->GetAttachmentHandle(false, dw), vector[0], vector[1], vector[2]);
						ves->GlobalRot(vector[2], vector[4]);
						d2 = dotp( vector[3], vector[4] );
						if (d2 < d ) return false;

						if (dw < 0 || dw >= ves->AttachmentCount(false) ||
							dw2 < 0 || dw2 >= ves2->AttachmentCount(true)) return true;
						CPylon::PylonAttach(ves->GetHandle(),
											ves2->GetHandle(),
											ves->GetAttachmentHandle(false,dw),
											ves2->GetAttachmentHandle(true,dw2));
						return true;
					}
					return false;

				/*
					RELEASE vessel vel
				*/
				case PYL_CMD_RELEASE:

					activate = false;
					ves = (VESSEL*)(value.tempRef);
					n = ves->AttachmentCount(true);
					i=0;
					obj2H = NULL;
					while (i<n && obj2H == NULL) {
						attH = ves->GetAttachmentHandle(true, i);
						obj2H = ves->GetAttachmentStatus(attH);
						i++;
					}
					if ( obj2H == NULL ) return true;

					ves2 = oapiGetVesselInterface(obj2H);

					n = ves2->AttachmentCount(false);
					i=0; objH = NULL;
					while (i<n && objH == NULL) {
						attH2 = ves2->GetAttachmentHandle(false, i);
						objH = ves2->GetAttachmentStatus(attH2);
						if (objH!=NULL) {
							if (ves->GetHandle() != objH) objH = NULL;
							else break;
						}
						i++;
					}
					if ( objH == NULL ) return true;

					// This does CTD in the call to orbiter detach.
					//CPylon::PylonDetach(obj2H, objH, attH, nextCmd->value.dblValue);

					return true;
				/*
					RELEASE_ATT parent_vessel att vel
				*/
				case PYL_CMD_RELEASE_ATT:
					activate = false;
					ves = (VESSEL*)(value.tempRef);

					i = nextCmd->value.intValue;
					n = ves->AttachmentCount(false);
					if (i>=n) return true;
					attH = ves->GetAttachmentHandle(false, i);
					if (!attH) return true;
					objH = ves->GetAttachmentStatus(attH);
					if (!objH) return true;
					CPylon::PylonDetach(ves->GetHandle(), objH, attH, nextCmd->nextCmd->value.dblValue, nextCmd->nextCmd->nextCmd->value.intValue != 0 );
					return true;

				/*
					CREATE_ATTACHED name(parent) int(tochild) int(toparent) name(child) classname(child)
				*/
				case PYL_CMD_CREATE_ATTACHED:
					ves = (VESSEL*)value.tempRef;
					objH = ves->GetHandle();

					dw = nextCmd->value.intValue;
					dw2 = nextCmd->nextCmd->value.intValue;

					if ( dw < 0 || dw >= ves->AttachmentCount(false) ) return true;
					if ( ves->GetAttachmentStatus( ves->GetAttachmentHandle(false, dw) ) ) return true;

					if ( activate ) {

						//vessel name
						c = nextCmd->nextCmd->nextCmd->value.strValue;
						if ( oapiGetVesselByName( c ) != 0 ) {

							i = 1;
							_snprintf_s(strTemp, NAME_SIZE, NAME_SIZE, "%s_%d", c, i);
							while ( oapiGetVesselByName( strTemp ) != 0 ) {
								i++;
								_snprintf_s(strTemp, NAME_SIZE, NAME_SIZE, "%s_%d", c, i);
							}

							c = strTemp;
						}

						//status1.version = 2;
						ves->GetGlobalPos(vector[0]);
						status.rpos = vector[0];
						status.rvel = _V(0,0,0);
						status.rbody = oapiGetGbodyByIndex( 0 );

						obj2H = (VESSEL*) oapiCreateVesselEx( c, (nextCmd->nextCmd->nextCmd->nextCmd->value.strValue), (void*)&status );
						ves2 = (VESSEL*) oapiGetVesselInterface(obj2H);

						value.tempIndex = (INT)obj2H;

						if (!ves2) return true;
						if (dw2 < 0 || dw2 >= ves2->AttachmentCount(true)) return true;

						nextCmd->nextCmd->nextCmd->value.tempIndex = 1;
						activate = false;
						return false;

					} else {

						if ( nextCmd->nextCmd->nextCmd->value.tempIndex ) {
							nextCmd->nextCmd->nextCmd->value.tempIndex--;
							return false;
						}

						obj2H = (VESSEL*) value.tempIndex;
						ves2 = oapiGetVesselInterface(obj2H);
						CPylon::PylonAttach(ves->GetHandle(),
											ves2->GetHandle(),
											ves->GetAttachmentHandle(false,dw),
											ves2->GetAttachmentHandle(true,dw2));
					}
					return true;

				/*
					DESTROY vessel
				*/
				case PYL_CMD_DESTROY:
					ves = (VESSEL*)(value.tempRef);

					objH = NULL;
					dw = 0;
					while (dw < ves->AttachmentCount(true) && !objH)
					{
						attH = ves->GetAttachmentHandle(true, dw);
						objH = ves->GetAttachmentStatus(attH);
						dw++;
					}

					if (objH)
					{
						CPylon::PylonDetach(objH, ves->GetHandle(), attH, 0);
					}

					oapiDeleteVessel(ves->GetHandle());

				return true;

				/*
					SET_FUEL vessel propellant_tank level
				*/
				case PYL_CMD_SET_FUEL:
					activate = false;
					ves = (VESSEL*)(value.tempRef);
					dw = nextCmd->value.intValue;

					if (dw >= ves->GetPropellantCount()) return true;
					propH = ves->GetPropellantHandleByIndex(dw);
					d = ves->GetPropellantMaxMass(propH) * nextCmd->nextCmd->value.dblValue;
					if (d == 0) return true;
					ves->SetPropellantMass(propH, d);
					return true;

				/*
					TRANSFER_FUEL srcvessel sourcetank destvessel desttank fuelmass
				*/
				case PYL_CMD_TRANSFER_FUEL:
					ves = (VESSEL*)(value.tempRef);
					dw = nextCmd->value.intValue;
					ves2 = (VESSEL*)(nextCmd->nextCmd->value.tempRef);
					dw2 = nextCmd->nextCmd->nextCmd->value.intValue;
                    //_snprintf_s(oapiDebugString(), NAME_SIZE, NAME_SIZE, "TRANSFER_FUEL (t=%f): %s:%d -> %s:%d", oapiGetSimTime(), ves->GetName(), dw, ves2->GetName(), dw2);

					if ( activate) {
						value.tempDbl = nextCmd->nextCmd->nextCmd->nextCmd->value.dblValue;
						activate = false;
					}
					if ( oapiGetSimStep() <= 0 || value.tempDbl <= 0 ) return true;
					if ( dw < 0 || dw >= ves->GetPropellantCount() ) return true;
					if ( dw2 < 0 || dw2 >= ves2->GetPropellantCount() ) return true;

					d = value.tempDbl;
					//d = nextCmd->nextCmd->nextCmd->nextCmd->nextCmd->value.dblValue * oapiGetSimStep();
					//if ( value.tempDbl < d ) {
					//  d = value.tempDbl;
					//}

					propH = ves->GetPropellantHandleByIndex(dw);
					d2 = ves->GetPropellantMass(propH);
					if ( d2 < d )
					{
						d = d2;
					}
					value.tempDbl -= d;
					ves->SetPropellantMass(propH, d2 - d);

					propH = ves->GetPropellantHandleByIndex(dw2);
					ves2->SetPropellantMass(propH, ves2->GetPropellantMass(propH) + d);

					ves3 = CPylon::GetRoot( ves );
					if ( ves3 == CPylon::GetRoot( ves2 ) ) {

                        if ( ves3 == ves ) {
                            // Source == root
                            ves3->SetEmptyMass( ves3->GetEmptyMass() + d );
                        }

                        if ( ves3 == ves2 ) {
                            // Dest == root
                            ves3->SetEmptyMass( ves3->GetEmptyMass() - d );
                        }

					}

					return true;

				/*
					SET_MAIN_THRUST vessel level thrustvel
		 		*/
				case PYL_CMD_SET_MAIN_THRUST:
					ves = (VESSEL*)(value.tempRef);

					if ( oapiGetSimStep() <= 0 || nextCmd->nextCmd->value.dblValue <= 0 ) return true;
					d = nextCmd->nextCmd->value.dblValue * oapiGetSimStep();

					d2 = ves->GetThrusterGroupLevel( THGROUP_MAIN );
					if ( d2 - nextCmd->value.dblValue == 0 ) return true;

					if ( d2 < nextCmd->value.dblValue )
					{
						d = d2 + d;
						if ( d > nextCmd->value.dblValue ) d = nextCmd->value.dblValue;
					}
					else
					if ( d2 > nextCmd->value.dblValue )
					{
						d = d2 - d;
						if ( d < nextCmd->value.dblValue ) d = nextCmd->value.dblValue;
					}
					ves->SetThrusterGroupLevel(THGROUP_MAIN, d);

					if ( d - nextCmd->value.dblValue != 0 ) return false;
					return true;

				/*
					SET_RETRO_THRUST vessel level thrustvel
				*/
				case PYL_CMD_SET_RETRO_THRUST:
					ves = (VESSEL*)(value.tempRef);

					if ( oapiGetSimStep() <= 0 || nextCmd->nextCmd->value.dblValue <= 0 ) return true;
					d = nextCmd->nextCmd->value.dblValue * oapiGetSimStep();

					d2 = ves->GetThrusterGroupLevel( THGROUP_RETRO );
					if ( d2 - nextCmd->value.dblValue == 0 ) return true;

					if ( d2 < nextCmd->value.dblValue )
					{
						d = d2 + d;
						if ( d > nextCmd->value.dblValue ) d = nextCmd->value.dblValue;
					}
					else
					if ( d2 > nextCmd->value.dblValue )
					{
						d = d2 - d;
						if ( d < nextCmd->value.dblValue ) d = nextCmd->value.dblValue;
					}
					ves->SetThrusterGroupLevel(THGROUP_RETRO, d);

					if ( d - nextCmd->value.dblValue != 0 ) return false;
					return true;

				/*
					SET_HOVER_THRUST vessel level thrustvel
				*/
				case PYL_CMD_SET_HOVER_THRUST:
					ves = (VESSEL*)(value.tempRef);

					if ( oapiGetSimStep() <= 0 || nextCmd->nextCmd->value.dblValue <= 0 ) return true;
					d = nextCmd->nextCmd->value.dblValue * oapiGetSimStep();

					d2 = ves->GetThrusterGroupLevel( THGROUP_HOVER );
					if ( d2 - nextCmd->value.dblValue == 0 ) return true;

					if ( d2 < nextCmd->value.dblValue )
					{
						d = d2 + d;
						if ( d > nextCmd->value.dblValue ) d = nextCmd->value.dblValue;
					}
					else
					if ( d2 > nextCmd->value.dblValue )
					{
						d = d2 - d;
						if ( d < nextCmd->value.dblValue ) d = nextCmd->value.dblValue;
					}
					ves->SetThrusterGroupLevel(THGROUP_HOVER, d);

					if ( d - nextCmd->value.dblValue != 0 ) return false;
					return true;

				/*
					SET_THRUSTER_LEVEL vessel thruster level thrustvel
				*/
				case PYL_CMD_SET_THRUST:
					activate = false;
					ves = (VESSEL*)(value.tempRef);
					dw = nextCmd->value.intValue;
					if (dw >= ves->GetThrusterCount()) return true;
					thrustH = ves->GetThrusterHandleByIndex(dw);
					ves->SetThrusterLevel(thrustH, nextCmd->nextCmd->value.dblValue);
					return true;

				/*
					CHANGE_THRUST_DIRECTION vessel thruster vessel
				*/
				case PYL_CMD_CHANGE_THRUST_DIRECTION:
					ves = (VESSEL*)(value.tempRef);
					dw = nextCmd->value.intValue;
					ves2 = (VESSEL*)(nextCmd->nextCmd->value.tempRef);

					if (dw < 0 || dw >= ves->GetThrusterCount()) return true;
					thrustH = ves->GetThrusterHandleByIndex(dw);
					if ( !thrustH ) return true;

					ves2->Local2Global(_V(0,0,0), vector[1]);
					ves->Global2Local(vector[1], vector[0]);
					ves->SetThrusterRef(thrustH, vector[0]);

					ves2->GlobalRot(_V(0,0,1), vector[1]);
					ves->GetRotationMatrix(matrix);
					vector[0] = tmul(matrix, vector[1]);
					ves->SetThrusterDir(thrustH, vector[0]);
					return false;


				/*
					SET_CAMERA_OFFSET vessel x y z
				*/
				case PYL_CMD_SET_CAMERA_OFFSET:
					activate = false;
					ves = (VESSEL*)(value.tempRef);
					ves->SetCameraOffset(
						_V(nextCmd->value.dblValue, nextCmd->nextCmd->value.dblValue,nextCmd->nextCmd->nextCmd->value.dblValue));
					return true;

                /*
					SET_CAMERA_DIRECTION vessel x y z tilt
				*/
				case PYL_CMD_SET_CAMERA_DIRECTION:
					activate = false;
					ves = (VESSEL*)(value.tempRef);
					ves->SetCameraDefaultDirection( unit( _V( nextCmd->value.dblValue,
                                                              nextCmd->nextCmd->value.dblValue,
                                                              nextCmd->nextCmd->nextCmd->value.dblValue ) ),
                                                              nextCmd->nextCmd->nextCmd->nextCmd->value.dblValue * PI / 180.0 );
                    oapiCameraSetCockpitDir( 0, 0 );
					return true;

				case PYL_CMD_CREATE_ATTACHMENT: return true;  // vessel id x y x dx dy dz rx ry rz loose(bool)
				case PYL_CMD_SET_TOCHILD_ATT: return true; // vessel attindex x y x dx dy dz rx ry rz loose(bool)

				/*
					SET_FOCUS_ENABLE vessel bool
				*/
				case PYL_CMD_SET_ENABLE_FOCUS:
					ves = (VESSEL*)(value.tempRef);
					ves->SetEnableFocus(nextCmd->value.bolValue);
					return true;

				// OrbiterSound interface

				/*
					SOUND_LOAD soundfile(str) soundname(str) soundslot(1..60) mode(0..8)
					Mode:
						DEFAULT, 0
						INTERNAL_ONLY, 1
						BOTHVIEW_FADED_CLOSE, 2
						BOTHVIEW_FADED_MEDIUM, 3
						BOTHVIEW_FADED_FAR, 4
						EXTERNAL_ONLY_FADED_CLOSE, 5
						EXTERNAL_ONLY_FADED_MEDIUM, 6
						EXTERNAL_ONLY_FADED_FAR, 7
						RADIO_SOUND, 8

				*/
				case PYL_CMD_SOUND_LOAD:
					i = nextCmd->nextCmd->value.intValue;
					if ( i < 1 || i > 60 ) return true;

					n = nextCmd->nextCmd->nextCmd->value.intValue;
					if ( n<0 || n > 8) return true;

					if ( RequestLoadVesselWave( ((CPylon*)this->owner)->GetOrbiterSoundId(), i, value.strValue, EXTENDEDPLAY(n) ) )
					{
						_snprintf_s(((CPylon*)this->owner)->soundNames[i-1].soundName, NAME_SIZE, NAME_SIZE, "%s", nextCmd->value.strValue );
						_snprintf_s(((CPylon*)this->owner)->soundNames[i-1].soundPath, NAME_SIZE, NAME_SIZE, "%s", value.strValue );
						((CPylon*)this->owner)->soundNames[i-1].mode = n;
					}
					return true;

				case PYL_CMD_SOUND_OPTION: return true; // option(int) onoff(bool)

				//soundname(soundname) loop(bool)=F volume(int_255)=255 freq(int)=0
				/*
					SOUND_PLAY soundname(str) loop(0,1) volume(0..255)
				*/
				case PYL_CMD_SOUND_PLAY:
					if (activate)
					{
						activate = false;
						i = nextCmd->nextCmd->value.intValue;
						if ( i < 0 ) i = 0; if ( i > 255 ) i = 255;
						nextCmd->nextCmd->value.intValue = i;

						i = 1;
						d = strcmp( ((CPylon*)this->owner)->soundNames[i-1].soundName, value.strValue ) != 0;
						while ( d && i < 60)
						{
							i++;
							d = strcmp( ((CPylon*)this->owner)->soundNames[i-1].soundName, value.strValue ) != 0;
						}
						if ( d != 0 ) return true;
						value.tempIndex = i;

						PlayVesselWave( ((CPylon*)this->owner)->GetOrbiterSoundId(), value.tempIndex,
												nextCmd->value.intValue?LOOP:NOLOOP,
												nextCmd->nextCmd->value.intValue );
						return false;
					}

					if ( nextCmd->value.intValue )
					{
						PlayVesselWave( ((CPylon*)this->owner)->GetOrbiterSoundId(), value.tempIndex,
												nextCmd->value.intValue?LOOP:NOLOOP,
												nextCmd->nextCmd->value.intValue );
						return false;
					}
					else
					{
						if ( IsPlaying(((CPylon*)this->owner)->GetOrbiterSoundId(), value.tempIndex ) ) return false;
					}
					return true;

				/*
					SOUND_STOP soundname(str)
				*/
				case PYL_CMD_SOUND_STOP:
					i = 1;
					d = strcmp( ((CPylon*)this->owner)->soundNames[i-1].soundName, value.strValue ) != 0;
					while ( d && i < 60)
					{
						i++;
						d = strcmp( ((CPylon*)this->owner)->soundNames[i-1].soundName, value.strValue ) != 0;
					}
					if ( d != 0 ) return true;
					StopVesselWave(((CPylon*)this->owner)->GetOrbiterSoundId(), i);
					return true; //soundname(soundname)

				/*
					SOUND_REPLACE_STOCK soundfile(str) WhichSoundToReplace(int)
				*/
				case PYL_CMD_SOUND_REPLACE_STOCK:
					ReplaceStockSound(((CPylon*)this->owner)->GetOrbiterSoundId(),value.strValue,nextCmd->value.intValue);
					return true;

				// Mesh handling

				/*
					ADD_MESH meshfile(str) meshname(str) visibilitymode(int) x y z
				*/
				case PYL_CMD_ADD_MESH:

					i = nextCmd->nextCmd->value.intValue;
					vector[0] = _V(	nextCmd->nextCmd->nextCmd->value.dblValue,
									nextCmd->nextCmd->nextCmd->nextCmd->value.dblValue,
									nextCmd->nextCmd->nextCmd->nextCmd->nextCmd->value.dblValue);

					if ( ! ((CPylon*)this->owner)->meshes )
					{
						((CPylon*)this->owner)->meshes = new PylonMesh();
					}
					pmesh = ((CPylon*)this->owner)->meshes;
					pmesh->Add( ((CPylon*)this->owner),
														(unsigned char *)value.strValue,
														(unsigned char *)nextCmd->value.strValue,
														i,
														vector[0]);
					return true;

				/*
					SET_MESH_VISIBILITY meshname(str) visibilitymode(int)
						MESHVIS_NEVER          0x00
						MESHVIS_EXTERNAL       0x01
						MESHVIS_COCKPIT        0x02
						MESHVIS_ALWAYS         (MESHVIS_EXTERNAL|MESHVIS_COCKPIT)
						MESHVIS_VC             0x04
						MESHVIS_EXTPASS        0x10
				*/
				case PYL_CMD_SET_MESH_VISIBILITY:
					if ( ! ((CPylon*)this->owner)->meshes )
					{
						return true;
					}
					pmesh = ((CPylon*)this->owner)->meshes->FindMeshName( value.strValue );
					if ( !pmesh )
					{
						return true;
					}
					pmesh->visMode = nextCmd->value.intValue;
					((CPylon*)this->owner)->SetMeshVisibilityMode( pmesh->meshIndex, pmesh->visMode );
					return true;

				/*
					CLEAR_MESHES
				*/
				case PYL_CMD_CLEAR_MESHES:
					((CPylon*)this->owner)->ClearMeshes();
					pmesh = ((CPylon*)this->owner)->meshes;
					if ( pmesh )
					{
						pmesh->Clear();
						delete pmesh;
						((CPylon*)this->owner)->meshes = 0;

					}
					return true;

				// CPylon common properties management

				/*
					USER_SEQ_ENABLED seq bool
				*/
				case PYL_CMD_USER_SEQ_ENABLED:
					p = (CPylon*)(value.tempRef);
					dw = value.tempIndex;
					p->SetUserSequence(dw, nextCmd->value.bolValue);
					return true;

				/*
					USER_RELEASE_ENABLED pylonname bool
				*/
				case PYL_CMD_USER_RELEASE_ENABLED:
					p = (CPylon*)(value.tempRef);
					p->userReleaseEnabled = nextCmd->value.bolValue;
					return true;

				/*
					USER_CREATE_ENABLED pylonname bool
				*/
				case PYL_CMD_USER_CREATE_ENABLED:
					p = (CPylon*)(value.tempRef);
					p->userCreateEnabled = nextCmd->value.bolValue;
					return true;

				/*
					USER_DESTROY_ENABLED pylonname bool
				*/
				case PYL_CMD_USER_DESTROY_ENABLED:
					p = (CPylon*)(value.tempRef);
					p->userDestroyEnabled = nextCmd->value.bolValue;
					return true;

				/*
					ACTUALIZE_NP_CHILDS_ENABLED pylonname bool
				*/
				case PYL_CMD_ACTUALIZE_NP_CHILDS_ENABLED:
					p = (CPylon*)(value.tempRef);
					p->actualizeNotPylonChilds = nextCmd->value.bolValue;
					return true;

				/*
					CAN_NAVIGATE pylonname bool
				*/
				case PYL_CMD_CAN_NAVIGATE:
					p = (CPylon*)(value.tempRef);
					p->canNavigate = nextCmd->value.bolValue;
					return true;

				/*
					USER_PARAMETERS_ENABLED pylonname bool
				*/
				case PYL_CMD_USER_PARAMETERS_ENABLED:
					p = (CPylon*)(value.tempRef);
					p->userParametersEnabled = nextCmd->value.bolValue;
					return true;
				/*
					CURRENT_MFD_PARAM pylonname integer
				*/
				case PYL_CMD_CURRENT_MFD_PARAM:
					p = (CPylon*)(value.tempRef);
					p->SetMFDSelectedParameter(nextCmd->value.intValue);
					return true;

				/*
					SET_CAMERA_FOV fov velocity
				*/
				case PYL_CMD_SET_CAMERA_FOV:
					if ( oapiGetSimStep() <= 0 ) return true;
					d = RAD * nextCmd->value.dblValue * oapiGetSimStep();

					d2 = oapiCameraAperture();
					d3 = 0.5 * RAD * value.dblValue;
					if ( d2 - d3 == 0 ) return true;

					if ( d2 < d3 ) {
						d = d2 + d;
						if ( d > d3 ) d = d3;
					}
					else {
						d = d2 - d;
						if ( d < d3 ) d = d3;
					}
					oapiCameraSetAperture(d);

					if ( d - d3 != 0 ) return false;
					return true;

				/*
					SET_FOCUS vessel
				*/
				case PYL_CMD_SET_FOCUS:
					activate = false;
					ves = (VESSEL*)(value.tempRef);
					oapiSetFocusObject( ves->GetHandle() );
					return true;

				/*
					SEND_KEY vessel key down lshift rshift lctrl rcotrl lalt salt
				*/
				case PYL_CMD_SEND_KEY:
					activate = false;
					ves = (VESSEL*)(value.tempRef);

					lShift = nextCmd->nextCmd->nextCmd->value.bolValue;
					rShift = nextCmd->nextCmd->nextCmd->nextCmd->value.bolValue;
					lCtrl = nextCmd->nextCmd->nextCmd->nextCmd->nextCmd->value.bolValue;
					rCtrl = nextCmd->nextCmd->nextCmd->nextCmd->nextCmd->nextCmd->value.bolValue;
                    lAlt = nextCmd->nextCmd->nextCmd->nextCmd->nextCmd->nextCmd->nextCmd->value.bolValue;
					rAlt = nextCmd->nextCmd->nextCmd->nextCmd->nextCmd->nextCmd->nextCmd->nextCmd->value.bolValue;

					modifiers = lShift || rShift || lCtrl || rCtrl || lAlt || rAlt;

					if ( modifiers ) {
                        ((CPylon*)this->owner)->setKeyMap( lShift, rShift, lCtrl, rCtrl, lAlt, rAlt );
					}

					ves->SendBufferedKey( nextCmd->value.intValue, nextCmd->nextCmd->value.bolValue, modifiers ? ((CPylon*)this->owner)->keyMap : NULL );

					return true;

                /*
					REDIRECT_KEYS vessel
				*/
				case PYL_CMD_REDIRECT_KEYS:
					activate = false;
					ves = (VESSEL*)(value.tempRef);

					if ( ves->GetHandle() == ((CPylon*)this->owner)->GetHandle() ) {
                        ((CPylon*)this->owner)->redirectKeysVesselHandle = NULL;
                        ((CPylon*)this->owner)->redirectKeysVesselName[ 0 ] = 0;
					}
					else {
                        ((CPylon*)this->owner)->redirectKeysVesselHandle = ves->GetHandle();
                        _snprintf_s( ((CPylon*)this->owner)->redirectKeysVesselName, NAME_SIZE, NAME_SIZE, "%s", ves->GetName() );
					}
					return true;

				default: return true;
			}
		}
}


bool PylonSequence::CastValue(int cmdType, TPylParamValue *val) {

	TPylParamValue *source;
	int sourceType;

//4
/*if (!mainSequence&&cmdType==19)
_snprintf_s(oapiDebugString(), NAME_SIZE, NAME_SIZE,
"CAST: selfvalt=%d,selfvalSTRv='%s',cmdt=%d, valt=%d, valstrvalue=%s",
value.type,value.strValue, cmdType,val?val->type:-1,val?val->strValue:"noval");
*/
	if (mainSequence) {

		if (value.type==PYL_PARAM_NOT_DEFINED) return true;
		else {
			source = val;
			sourceType = val->type;
			cmdType = value.type;
		}
	} else {
		if (value.type==PYL_PARAM_NOT_DEFINED) {
			source = &(((PylonSequence *)caller)->value);
			sourceType = source->type;
		} else {
			source = &value;
			sourceType = value.type;
		}
	}
	if (cmdType == PYL_PARAM_NOT_DEFINED) cmdType = sourceType;
	if (sourceType < 0) {
		char *c,*cc,tempstr[NAME_SIZE];
		int i,n;
		OBJHANDLE oh; CPylon* pylon;
		switch (sourceType) {
			case PYL_PARAM_GET_PARAMETER:
				c = name; cc = &tempstr[0];
				while (*c!=0 && *c!=':') { *cc++ = *c++;}
				if (*c==0) return false;
				*cc = 0; c++; if (*c==0) return false;
				if (cc==tempstr) pylon = (CPylon*)owner;
				else {
					oh = oapiGetVesselByName(tempstr);
					pylon = CPylon::IsPylonVessel(oapiGetVesselInterface(oh));
				}
				if (pylon == NULL) return false;
				i=0; n = pylon->GetParameterCount();
				while (i<n && strcmp(c, pylon->GetParameterName(i))!=0) {
					i++;
				}
				if (i==n) return false;

				sourceType = pylon->GetParameterType(i);
				if (sourceType < PYL_PARAM_INTEGER)
					value.bolValue = pylon->GetParamBol(i);
				else if (sourceType < PYL_PARAM_SCALAR)
					value.intValue = pylon->GetParamInt(i);
				else if (sourceType < PYL_PARAM_STRING)
					value.dblValue = pylon->GetParamDbl(i);
				else if (sourceType < PYL_PARAM_NOT_DEFINED)
					_snprintf_s(value.strValue, NAME_SIZE, NAME_SIZE, pylon->GetParamStr(value.tempIndex));
				else return false;
			break;

		//PYL_PARAM_GENERATE_UNIFORM = -2; // symbol '_U'. Has two parameters, min and max (double)
		//sourceType = PYL_PARAM_SCALAR;

		//PYL_PARAM_GENERATE_NORMAL = -3; // symbol '_N'. Has two parameters, mean and stddesv (double)
		//otros para leer cosas del pylon: thruster level con thrhandle, fuel level, ..
		//sourceType = PYL_PARAM_SCALAR;

		// PYL_PARAM_THIS_VESSEL
		// (cmdType==PYL_PARAM_PARAMETER_NAME)
		//	sourceType = PYL_PARAM_PARAMETER_NAME
		//
		//if (cmdType==PYL_PARAM_PARAMETER_NAME)
		//	sourceType = PYL_PARAM_PARAMETER_NAME

		// return true normalmente, o false en '?' no valido
		//el ?param ha de estar en name, creado con new.
		default: return false;
		}
		return ConvertValue(cmdType, sourceType, source, true);
	}
	if (source == &value) {
/*if (!mainSequence&&cmdType==19)
_snprintf_s(oapiDebugString(), NAME_SIZE, NAME_SIZE,
"CAST4: selfvalt=%d,selfvalSTRv='%s',cmdt=%d, sorcetype=%d,sourcestrval='%s'",
value.type,value.strValue,
cmdType,sourceType,source->strValue);
*/
		if (cmdType > PYL_PARAM_STRING && cmdType < PYL_PARAM_NOT_DEFINED) {
			return Validate(cmdType, sourceType);
		}
		else return true;
	}

	bool b = ConvertValue(cmdType, sourceType, source, true);
/*if (!mainSequence&&cmdType==19)
_snprintf_s(oapiDebugString(), NAME_SIZE, NAME_SIZE,
"CAST5: selfvalt=%d,selfvalSTRv='%s',cmdt=%d, sorcetype=%d,sourcestrval='%s', convert:%d",
value.type,value.strValue,
cmdType,sourceType,source->strValue,b?1:0);
*/
	return b;//ConvertValue(cmdType, sourceType, source, true);
}

bool PylonSequence::ConvertValue(int destType, int sourceType, TPylParamValue *source, bool doValidate) {
	bool conv = source!=&value;

//5
/*if (!mainSequence&&destType==0)
_snprintf_s(oapiDebugString(), NAME_SIZE, NAME_SIZE, "convertValue:  dtyp %d styp %d dblval= %f,conv=%d",
		destType, sourceType, source->dblValue,conv?1:0);
*/
	// Boolean
	if (destType < PYL_PARAM_INTEGER) {
		if (sourceType < PYL_PARAM_INTEGER) { if (conv) value.bolValue = source->bolValue; }
		else if (sourceType < PYL_PARAM_SCALAR) value.bolValue = source->intValue!=0;
		else if (sourceType < PYL_PARAM_STRING) value.bolValue = source->dblValue==0?false:true;
		else if (sourceType < PYL_PARAM_NOT_DEFINED) value.bolValue = source->strValue[0]!=0;
		if (!conv) value.type = destType;
		return true;
	}
	// Integer
	if (destType < PYL_PARAM_SCALAR) {
		if (sourceType < PYL_PARAM_INTEGER) value.intValue = source->bolValue;
		else if (sourceType < PYL_PARAM_SCALAR) { if (conv) value.intValue = source->intValue; }
		else if (sourceType < PYL_PARAM_STRING) value.intValue = (int)source->dblValue;
		else if (sourceType < PYL_PARAM_NOT_DEFINED) value.intValue = atoi(source->strValue);
		if (!conv) value.type = destType;
		return true;
	}
	// Scalar (double) types:
	if (destType < PYL_PARAM_STRING) {
		if (sourceType < PYL_PARAM_INTEGER) value.dblValue = source->bolValue;
		else if (sourceType < PYL_PARAM_SCALAR) value.dblValue = source->intValue;
		else if (sourceType < PYL_PARAM_STRING) { if (conv) value.dblValue = source->dblValue; }
		else if (sourceType < PYL_PARAM_NOT_DEFINED) value.dblValue = atof(source->strValue);
		if (!conv) value.type = destType;
		return true;
	}
	// String types:
	if (destType < PYL_PARAM_NOT_DEFINED) {
		if (sourceType < PYL_PARAM_INTEGER) _snprintf_s(value.strValue, NAME_SIZE, NAME_SIZE, "%s", &PylonBoolParamTypeName[sourceType][source->bolValue][0]);
		else if (sourceType < PYL_PARAM_SCALAR) _snprintf_s(value.strValue, NAME_SIZE, NAME_SIZE, "%d", source->intValue);
		else if (sourceType < PYL_PARAM_STRING) _snprintf_s(value.strValue, NAME_SIZE, NAME_SIZE, "%f", source->dblValue);
		else if (sourceType < PYL_PARAM_NOT_DEFINED) {
			if (conv) strncpy_s(value.strValue, NAME_SIZE, source->strValue, NAME_SIZE);

//5if (!mainSequence&&destType==19)
//_snprintf_s(oapiDebugString(),NAME_SIZE, NAME_SIZE, "convertValue2:  dtyp %d styp %d strval=%s,conv=%d,valuetype=%d,valuestrvalue=%s",
//		destType, sourceType, source->strValue,conv?1:0,value.type,value.strValue);

			if (destType > PYL_PARAM_STRING && doValidate) {
				bool validated=Validate(destType, sourceType);
//5if (!mainSequence&&destType==19)
//_snprintf_s(oapiDebugString(),NAME_SIZE, NAME_SIZE, "convertValue3:  dtyp %d styp %d strval=%s,conv=%d,valuetype=%d,valuestrvalue=%s, validated=%d",
//		destType, sourceType, source->strValue,conv?1:0,value.type,value.strValue,validated?1:0);

				return validated;
			}
			else return true;
		}
	}
	return false;
}

bool PylonSequence::Validate(int destType, int srcType) {
	if (mainSequence) return false;
//5_snprintf_s(oapiDebugString(),NAME_SIZE, NAME_SIZE, "Validate: cmdt:%d,  srctype:%d, valuestrvalue=%s",destType,srcType,value.strValue);
	if (destType == PYL_PARAM_NOT_DEFINED) destType = srcType;
	if (destType > PYL_PARAM_STRING && destType < PYL_PARAM_NOT_DEFINED) {

		if (srcType < PYL_PARAM_STRING || srcType >= PYL_PARAM_NOT_DEFINED) {
			return false;
		}
		OBJHANDLE oh;
		int i,n;
		char *c, *cc;
		char tempstr[NAME_SIZE];
		switch (destType) {
			case PYL_PARAM_OBJECT_NAME:
				value.tempH = oapiGetObjectByName(value.strValue);
				return value.tempH != NULL;
				break;
			case PYL_PARAM_VESSEL_NAME:
				oh = oapiGetVesselByName(value.strValue);
				if (oh==NULL) return false;
				value.tempRef = oapiGetVesselInterface(oh);
				return value.tempRef != NULL;
				break;
			case PYL_PARAM_VESSEL_CLASS:
				return true;
				break;
			case PYL_PARAM_GBODY_NAME:
				value.tempH = oapiGetGbodyByName(value.strValue);
				return (value.tempH!=NULL);
				break;
			case PYL_PARAM_BASE_NAME:
//				value.tempH = oapiGetBaseByName(value.strValue); -->> nomplaneta:nombase
				return false;//(value.tempH!=NULL);
				break;
			case PYL_PARAM_PARAMETER_NAME:
				c = value.strValue; cc = &tempstr[0];
				while (*c!=0 && *c!=':') { *cc++ = *c++;}

				if (*c==0) { //return false;
					value.tempRef = (void *)owner;
					c = value.strValue;
				} else {
					*cc = 0; c++; if (*c==0) return false;
					if (cc==tempstr) value.tempRef = owner;
					else {
						oh = oapiGetVesselByName(tempstr);
						if (oh==NULL) return false;
						value.tempRef = (void *)CPylon::IsPylonVessel(oapiGetVesselInterface(oh));
					}
				}//

/*				if (*c==0) return false;
				*cc = 0; c++; if (*c==0) return false;
				if (cc==tempstr) value.tempRef = owner;
				else {
					oh = oapiGetVesselByName(tempstr);
					if (oh==NULL) return false;
					value.tempRef = (void *)CPylon::IsPylonVessel(oapiGetVesselInterface(oh));
				}
*/
				if (value.tempRef == NULL) return false;
				i=0; n = ((CPylon*)value.tempRef)->GetParameterCount();
				while (i<n && strcmp(c, ((CPylon*)value.tempRef)->GetParameterName(i))!=0) {
					i++;
				}
				if (i==n) return false;
				value.tempIndex = i;
				return true;
				break;
			case PYL_PARAM_SEQUENCE_NAME:
				c = value.strValue; cc = &tempstr[0];
				while (*c!=0 && *c!=':') { *cc++ = *c++;}
				if (*c==0) return false;
				*cc = 0; c++; if (*c==0) return false;
				if (cc==tempstr) value.tempRef = owner;
				else {
					oh = oapiGetVesselByName(tempstr);
					if (oh==NULL) return false;
					value.tempRef = (void *)CPylon::IsPylonVessel(oapiGetVesselInterface(oh));
				}
				if (value.tempRef == NULL) return false;

				i=0, n = ((CPylon*)value.tempRef)->GetSequenceCount();
				while (i<n && strcmp(c, ((CPylon*)value.tempRef)->GetSequenceName(i))!=0)
					i++;
				if (i==n) return false;
				value.tempIndex = i;
				return ((CPylon*)value.tempRef)->IsSequenceValid(i);
				break;
			case PYL_PARAM_PYLON_NAME:
				oh = oapiGetVesselByName(value.strValue);
				if (oh==NULL) return false;
				value.tempRef = CPylon::IsPylonVessel(oapiGetVesselInterface(oh));
				return value.tempRef != NULL;
				break;

		}
	}
	return true;
}

char *PylonSequence::GetToken(char *line, TPylParamValue &value) {
	if (line==NULL) return NULL;
	while (*line!=0 && (*line==' ' || *line==9)) line++;
	if (*line==0) return NULL;

	if (*line=='#') { value.type = PYL_PARAM_NOT_DEFINED; return ++line; }
	if (*line=='?') { value.type = PYL_PARAM_GET_PARAMETER; return ++line; }
	if (*line=='(') { value.type = PYL_PARAM_TOKEN_OPENBR; return ++line; }
	if (*line==')') { value.type = PYL_PARAM_TOKEN_CLOSEBR; return ++line; }
	if (*line==',') { value.type = PYL_PARAM_TOKEN_COMMA; return ++line; }

	if (*line=='"') {
		value.type = PYL_PARAM_STRING;
		line++;
		char *p = value.strValue;
		while (*line!=0 && *line!='"') {
			while (*line!=0 && *line!='"') { *p = *line; p++; line++; }
			if (*line!=0 && *(line+1) == '"') { *p='"'; p++; line+=2; }
		}
		*p = 0;
		if (*line!=0) line++;
		return line;
	}

	char *end = line;
	char s[NAME_SIZE];
	int c = 0;
	while (c < (NAME_SIZE-1) && *end!=0 && *end!=' ' && *end!=9 && *end!='(' && *end!=')' && *end!=',') {
		s[c] = *end;
		c++;
		end++;
	}
	s[c] = 0;

	if ((*line>='0' && *line<='9') || *line=='.' || *line=='+' || *line=='-') {
		value.type = PYL_PARAM_SCALAR;
		value.dblValue = atof(s);
		return end;
	}

	if (*line=='_') {
		line++;
		if (*line=='U') {
			value.type = PYL_PARAM_GENERATE_UNIFORM; return ++line;
		}
		if (*line=='N') {
			value.type = PYL_PARAM_GENERATE_NORMAL; return ++line;
		}

		if (strcmp("THIS\0",s)==0) {
			value.type = PYL_PARAM_THIS; return ++line;
		}
		return NULL;
	}

	for (int i=0;i<PYL_CMD_NOT_DEFINED;i++) {
		if (strcmp(PylonCommands[i].name,s)==0) {
			value.type = PYL_PARAM_TOKEN_COMMAND;
			value.intValue = i;
			return end;
		}
	}
	if (strcmp("BEGINSEQ",s)==0) {
		value.type = PYL_PARAM_TOKEN_BEGINSEQ;
		return end;
	}
	if (strcmp("ENDSEQ",s)==0) {
		value.type = PYL_PARAM_TOKEN_ENDSEQ;
		return end;
	}
	if (strcmp("PARALLEL",s)==0) {
		value.type = PYL_PARAM_TOKEN_PARALLEL;
		return end;
	}
	if (strcmp("WAITONE",s)==0) {
		value.type = PYL_PARAM_TOKEN_WAITONE;
		return end;
	}
	if (strcmp("USER",s)==0) {
		value.type = PYL_PARAM_TOKEN_USER;
		return end;
	}

	return NULL;
}

void PylonSequence::Terminate(void) {
	if (mainSequence) {
		PylonSequence *ps = nextCmd;
		if (mode > SEQ_MODE_SEQUENTIAL) {
			if (ps && ps->value.type == PYL_CMD_SET_CANCEL_SEQUENCE)
				ps = ps->nextSeq;

			while (ps!=NULL) {
				if (!ps->finishedcommand) ps->Terminate();
				ps = ps->nextSeq;
			}
		} else {
			if (cmdcurrent!=NULL) cmdcurrent->Terminate();
		}
	} else {
		if (cmdcode==PYL_CMD_CALL_SEQ) {
			CPylon *p = (CPylon *)value.tempRef;
			PylonSequence *ps = p->sequences;
			int i=value.tempIndex;

			while (ps!=NULL && i>0) { i--; ps = ps->nextSeq; }

			if (ps!=NULL) ps->Terminate();
		}
	}

	finishedcommand = true;
	finished = true;
	activate = false;
	cancel = false;
	activating = false;
	cancelling = false;
}
