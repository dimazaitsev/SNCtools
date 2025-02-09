// SN obtained from NDR
#define m 10
#define n 12
// incoming arcs of transitions
static int b[10][12]=
{
{1,0,0,0,0,0,1,0,0,1,0,0},
{0,1,0,0,0,1,0,1,0,0,0,0},
{0,0,1,1,1,0,0,0,0,0,0,0},
{0,0,0,0,0,0,0,0,1,0,1,1},
{1,1,1,0,0,0,0,0,0,0,1,0},
{0,0,0,1,0,0,0,1,1,1,0,0},
{0,0,0,0,1,1,1,0,0,0,0,1},
{0,0,0,0,0,0,0,0,0,0,0,0},
{0,0,0,0,0,0,0,0,0,0,0,0},
{0,0,0,0,0,0,0,0,0,0,0,0}
};
// outgoing arcs of transitions
static int d[10][12]=
{
{0,0,0,1,0,1,1,0,1,1,0,0},
{1,0,0,0,1,0,0,1,0,0,0,0},
{0,1,0,0,0,0,0,0,0,0,0,1},
{0,0,1,0,0,0,0,0,0,0,1,0},
{0,0,0,0,0,0,0,0,0,0,0,0},
{0,0,0,0,0,0,0,0,0,0,0,0},
{0,0,0,0,0,0,0,0,0,0,0,0},
{0,0,0,0,1,1,0,0,0,0,1,1},
{0,0,0,1,0,0,0,0,0,0,0,0},
{0,0,0,0,0,0,0,0,1,0,0,0}
};
// priority arcs connecting transitions
static int r[12][12]=
{
{0,0,0,0,0,0,0,0,0,0,0,0},
{0,0,0,0,0,0,0,0,0,0,0,0},
{0,0,0,0,0,0,0,0,0,0,0,0},
{0,0,0,0,0,0,0,0,0,0,0,0},
{0,0,0,0,0,0,0,0,0,0,0,0},
{0,0,0,0,0,0,0,0,0,0,0,0},
{0,0,0,0,0,0,0,0,0,0,0,0},
{0,0,0,0,0,0,0,0,0,0,0,0},
{0,0,0,0,0,0,0,0,0,0,0,0},
{0,0,0,0,0,0,0,0,0,0,0,0},
{0,0,0,0,0,0,0,0,0,0,0,0},
{0,0,0,0,0,0,0,0,0,0,0,0}
};
// initial marking
static int mu[10]={1,0,0,0,0,0,0,0,0,0};
// Table of places
// no	name
// 0	q0
// 1	q1
// 2	q2
// 3	q3
// 4	coin
// 5	choc
// 6	ret
// 7	r_coin
// 8	m_choc
// 9	d_choc
// Table of transitions
// no	name
// 0	t0
// 1	t1
// 2	t2
// 3	t7
// 4	t9
// 5	t10
// 6	t12
// 7	t13
// 8	t6
// 9	t11
// 10	t5
// 11	t8
// end of SN

//SN pin map

#define MI 3
#define MO 3

#define DEBOUNCE_DELAY 50
static int in_places[MI]={4,5,6};
static int in_pins[MO]={15,14,13};
volatile static int in_pins_debounce_prev_time[MO]={0,0,0};
volatile static int in_mu[MI]={0,0,0};
volatile static int pi=0;
volatile static int dbt=0;

static int out_places[MO]={7,8,9};
static int out_pins[MO]={16,17,18};

void IR_Read_Gen() {
  dbt = millis();
  if((dbt - in_pins_debounce_prev_time[pi]) > DEBOUNCE_DELAY) {
    in_mu[pi]=1;
    go=1;
    in_pins_debounce_prev_time[pi]=dbt;
  }
  digitalWrite(16,LOW);
  digitalWrite(17,LOW);
  digitalWrite(18,LOW);
}
void IR_Read_0() { pi=0; IR_Read_Gen(); }
void IR_Read_1() { pi=1; IR_Read_Gen(); }
void IR_Read_2() { pi=2; IR_Read_Gen(); }

void Init_pins_IR()
{
  int i;
  for(i=0;i<MI;i++)
    pinMode(in_pins[i], INPUT_PULLUP);
  for(i=0;i<MO;i++){
    pinMode(out_pins[i], OUTPUT);
    digitalWrite(out_pins[i],LOW);
  }
  attachInterrupt(digitalPinToInterrupt(in_pins[0]), IR_Read_0, RISING);
  attachInterrupt(digitalPinToInterrupt(in_pins[1]), IR_Read_1, RISING);
  attachInterrupt(digitalPinToInterrupt(in_pins[2]), IR_Read_2, RISING);
}

void ReadIn()
  {
  int pi;
  for(pi=0;pi<MI;pi++)
    if(in_mu[pi]>0)
    {
      mu[in_places[pi]]=in_mu[pi];
      in_mu[pi]=0;
    }
}

void WriteOut()
{
  int po;
  for(po=0;po<MO;po++)
    if(mu[out_places[po]]>0)
    {
      digitalWrite(out_pins[po],(mu[out_places[po]]>1)?LOW:HIGH);
      mu[out_places[po]]=0;
    }
}

// end of SN pin map


