#include "OLED.h" 
#include "font.c"

#define WriteCmd(client, cmd)  OLED_WriteByte(client, 0x00, cmd)
#define WriteDat(client, data) OLED_WriteByte(client, 0x40, data)
#define send_Cmd(cmd)   WriteCmd(gloabl_client, cmd)
#define send_Data(data) WriteDat(gloabl_client, data)

static struct i2c_client *gloabl_client;
const  uint8_t* m_font;             //  当前使用字体
static uint8_t m_font_offset = 2;  //  字间距
static uint8_t m_font_width;       //  字宽
static uint8_t current_x;          // 当前的x
static uint8_t current_y;          // 当前的y



static void OLED_WriteByte(struct i2c_client *client, uint8_t addr, uint8_t data)
{
	uint8_t buf[] = {addr, data};
	i2c_master_send(client, buf, 2);
}

void OLED_setFont(const uint8_t* font)
{
  m_font = font;
  m_font_width = pgm_read_byte(&m_font[0]);
}

static uint8_t DispalyBuffer[128][8];   
/*                        
这里我们把128*64 划分为128*8个block（uint8_t），每一个block 描述了竖向的
8个像数点情况，这样横向我们有128个block，然后我们竖向每8个点正好是一个（uint8_t）
来描述，所以我们有8个（uint_t）竖向的，因此我们的显存就是128*8的大小
*/

//清屏
void OLED_clear_screen(uint8_t chFill)
{
    memset(DispalyBuffer,chFill, sizeof(DispalyBuffer));    //置为统一的0或1
    current_x = 0;
    current_y = 56;
    OLED_refresh_gram();                                    // 显示处理
}
//显示刷新
void OLED_refresh_gram(void)
{
  uint8_t i, j;
  for (i = 0; i < 8; i ++)   //按照竖向的8个来发
  {
	send_Cmd(0xB0+i);
    send_Cmd(0x02);
    send_Cmd(0x10);
    for (j = 0; j < 128; j ++) 
      send_Data(DispalyBuffer[j][i]);  //一个横向有128个（uint8_t）描述竖向的8个像数
  }
}
//画点（以一个Uint8 的一个位为对象）
int OLED_draw_point(uint8_t chXpos, uint8_t chYpos, uint8_t chPoint)
{
    uint8_t chPos, chBx, chTemp = 0;
    if (chXpos > 127 || chYpos > 63)
    {
        return -1;
    }
	chPos = 7 - chYpos / 8;    //得到是在竖向的哪个block的区间
    chBx = chYpos % 8;         //具体block中的哪一个点
    chTemp = 1 << (7 - chBx);   //位运算获取那个点

    if (chPoint) 
        DispalyBuffer[chXpos][chPos] |= chTemp;   //对对应位进行置位
	else 
        DispalyBuffer[chXpos][chPos] &= ~chTemp;  //对对应位进行置位
    
    return 0;
}
//输出一个字符
int OLED_putChar(uint8_t chXpos, uint8_t chYpos, uint8_t chChr, uint8_t chMode)
{
	uint8_t i, j,ret;
    uint8_t chTemp, chYpos0 = chYpos;    //基准y坐标

    chChr = chChr - ' ';           // 获取在ASCII中的位置
    for (i = 0; i < m_font_width; i ++) {
        if (chMode) //
            chTemp = m_font[2 + chChr*m_font_width + i];    //获取在字库的位置
        else 
            chTemp = ~m_font[2 + chChr*m_font_width + i];
        for (j = 0; j < 8; j ++) 
		{
            if (chTemp & 0x80) 
                ret = OLED_draw_point(chXpos, chYpos++, 1);
            else 
                ret = OLED_draw_point(chXpos, chYpos++, 0);
            if(ret)
                return -1; 
            chTemp <<= 1;
            if ((chYpos - chYpos0) == 7) {   //chsize
                chYpos = chYpos0;
                chXpos ++;
                break;
            }
        }
    }
    return 0;
}
//输出字符串
//从上向下输出
int OLED_putString(uint8_t chXpos, int8_t chYpos,const char *string,uint8_t chMode)
{
    uint8_t ret;
	while (*string != '\0') 
	{
		if (chXpos > (OLED_WIDTH - m_font_width / 2)) 
		{
            chXpos = 0;
            chYpos -= 8;
            if (chYpos <= 0) 
			{
                chYpos = 56;
                chXpos = 0;
                OLED_clear_screen(0x00);
            }
        }
        ret = OLED_putChar(chXpos, chYpos, *string, chMode);
        if(ret)
            return -1;
        chXpos += m_font_width - m_font_offset;
        string ++;
	 }
     current_x = chXpos;
     current_y = chYpos;
     return 0;
}


int OLED_addString(const char *string, uint8_t chMode)
{
    int ret;
    ret =  OLED_putString(current_x,current_y,string,chMode);
    if(ret)
    {
        //do something
        return 1;
    }
    return 0;
}
int  get_gram_OLED_string(const char *string)
{
    const char * str = string;
    int i = 0;
    uint8_t blank_count= 0;
    uint8_t pos[2] = {0};
    str = str + 2;
    while(blank_count!=2)
    {
        while (*str != ' ')
        {
            if(*str >= '0' && *str <= '9' )
            {
                pos[i] = pos[i] * 10;
                pos[i] += *str - '0';
            }
            str++;  
        }
        blank_count++;
        i++;  //两个参数  
        str++;
    }
    if(pos[0]>= 0 && pos[0] <=128)
    {
        if(pos[1]>= 0 && pos[1] <=56)
        {
            OLED_putString(pos[0],pos[1],str,1);
        }
    }
    OLED_putString(0,56,"error in gram",1);
    return 0;
}
int abs_usr(int x)
{
    if(x >= 0) 
        return x;
    else 
        return -x;
}

int OLED_dram_line(const char *string)
{
    const char * str = string;
    int i = 0;
    uint8_t blank_count= 0;
    uint8_t pos[4] = {0};  
    str = str + 2;
    while(blank_count!=4)
    {
        while (*str != ' ')
        {
            if(*str >= '0' && *str <= '9' )
            {
                pos[i] = pos[i] * 10;
                pos[i] += *str - '0';
            }
            str++;  
        }
        blank_count++;
        i++;  //四个参数  
        str++;
    }
    if(pos[0]>= 0 && pos[0] <=128 && pos[2] >= 0 && pos[2] <= 128)
    {
        if(pos[1]>= 0 && pos[1] <=64 && pos[3] >= 0 && pos[3] <=64)
        {
            char distan_y =  pos[3] - pos[1] >= 0 ? 1:-1;
            char distan_x =  pos[2] - pos[0] >= 0 ? 1:-1;
            int divde_segm = abs_usr(pos[3] - pos[1]);
            int length = abs_usr(pos[2] - pos[0]);
            int j ;
            if(divde_segm <= 1 )
            {
                for ( i = 0; i <= length; i++)
                    OLED_draw_point(pos[0]+i*distan_x,pos[1],1);
                return 0;
            }
            else if (length == 0 && divde_segm != 0)
            {
                for ( i = 0; i <= divde_segm; i++)
                    OLED_draw_point(pos[0],pos[1]+i*distan_y,1);
            }
            else
            {
                int add_length = length/ divde_segm;
                int draw_x = pos[0];
                int draw_y = pos[1];
                for ( i = 0; i < divde_segm; i++)
                {
                    for (j = 0 ; j < add_length ; j++)
                    {
                        OLED_draw_point(draw_x,draw_y,1);
                        draw_x +=  distan_x;
                    }
                    draw_y +=  distan_y ;
                }
                return 0;
            }
        }
    }
    return -1;
}


int OLED_draw_circle(const char *string)
{
    const char * str = string;
    int i = 0;
    uint8_t blank_count= 0;
    uint8_t pos[3] = {0};   //圆心 半径R
    str = str + 2;
    while(blank_count!=3)
    {
        while (*str != ' ')
        {
            if(*str >= '0' && *str <= '9' )
            {
                pos[i] = pos[i] * 10;
                pos[i] += *str - '0';
            }
            str++;  
        }
        blank_count++;
        i++;  //两个参数  
        str++;
    }
    if(pos[0]> 0 && pos[0] <128)
    {
        if(pos[1]> 0 && pos[1] <64)
        {
            OLED_putString(pos[0],pos[1],str,1);
        }
    }
    OLED_putString(pos[0],pos[1],str,1);
    return 0;
}
int divide_Cmd(const char *string)
{
    const char * str = string;
    char flag = str[0];
    switch (flag)
    {
    case 'a': OLED_addString(str+2,1);
        /* code */
        break;
    case 'n': get_gram_OLED_string(str);
        break;
    case 'l' :OLED_dram_line(str);
        break;
    case 'c': OLED_clear_screen(0x00);
        break;
    default:
        break;
    }
    return 0;
}

int OLED_Probe(struct i2c_client *client, const struct i2c_device_id *id)
{
  	WriteCmd(client, 0xAE); //display off
	WriteCmd(client, 0x20);	//Set Memory Addressing Mode	
  	WriteCmd(client, 0x10);	//00,Horizontal Addressing Mode;01,Vertical Addressing Mode;10,Page Addressing Mode (RESET);11,Invalid
	WriteCmd(client, 0xb0);	//Set Page Start Address for Page Addressing Mode,0-7
	WriteCmd(client, 0xc8);	//Set COM Output Scan Direction
	WriteCmd(client, 0x00); //---set low column address
	WriteCmd(client, 0x10); //---set high column address
	WriteCmd(client, 0x40); //--set start line address
	WriteCmd(client, 0x81); //--set contrast control register
	WriteCmd(client, 0xff); //亮度调节 0x00~0xff
	WriteCmd(client, 0xa1); //--set segment re-map 0 to 127
	WriteCmd(client, 0xa6); //--set normal display
	WriteCmd(client, 0xa8); //--set multiplex ratio(1 to 64)
	WriteCmd(client, 0x3F); //
	WriteCmd(client, 0xa4); //0xa4,Output follows RAM content;0xa5,Output ignores RAM content
	WriteCmd(client, 0xd3); //-set display offset
	WriteCmd(client, 0x00); //-not offset
	WriteCmd(client, 0xd5); //--set display clock divide ratio/oscillator frequency
	WriteCmd(client, 0xf0); //--set divide ratio
	WriteCmd(client, 0xd9); //--set pre-charge period
	WriteCmd(client, 0x22); //
	WriteCmd(client, 0xda); //--set com pins hardware configuration
	WriteCmd(client, 0x12);
	WriteCmd(client, 0xdb); //--set vcomh
	WriteCmd(client, 0x20); //0x20,0.77xVcc
	WriteCmd(client, 0x8d); //--set DC-DC enable
	WriteCmd(client, 0x14); //
	gloabl_client = client;
	//OLED_Cls(client);
	OLED_clear_screen(0x00);
	WriteCmd(client, 0xaf); //--turn on oled panel

	return 0;
}
int OLED_Remove(struct i2c_client *client)
{
	printk("remove\n");
	WriteCmd(client, 0xAE);
	return 0;
}
