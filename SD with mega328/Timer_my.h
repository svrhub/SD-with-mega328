/*
 * Timer.h
 *
 * Created: 2016/7/2 11:42:23
 *  Author: Stone
 */ 


#ifndef TIMER_H_
#define TIMER_H_


void Timer0_clock()
{
	TCNT0=0x00;
	OCR0  = 0x80;
	TIMSK = 0x01;
	TCCR0=(1<<WGM01)|(1<<WGM00)|(1<<COM01)|(1<<COM00)|(1<<CS00)|(1<<CS01);
	sei();
	//TIFR=(1<<TOV0);
	//TIMSK=(1<<TOIE1);	  //使能T1计时器溢出中断
}
ISR(TIMER0_OVF_vect){
	//TIFR=(1<<TOV0);
	TCNT0=0x10;
}
void Timer1_clock(unsigned char cmd)
{	
	sei();
	if (cmd==1)
	{	
		PORTG|=(1<<PORTG4);	
		TCCR1B|=(1<<WGM12)|(1<<CS12)|(1<<CS10);		//CTC比较模式，1024分频
		TCNT1=0;									
		OCR1A=0x0F42;								//CTC比较上限值
		TIFR=(1<<OCF1A);							//清中断位
		TIMSK|=(1<<OCIE1A);							//使能T1计时器比较中断		
	}else{
		TCCR1B=0;
	}	
}

ISR(TIMER1_COMPA_vect)
{	
	PORTG^=(1<<PORTG4);
}


#endif /* TIMER_H_ */