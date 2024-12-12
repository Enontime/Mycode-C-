import matplotlib.pyplot as plt
import matplotlib.patches as patches

class Ordinal:
    def __init__(self, name, elements):
        self.name = name
        self.elements = elements
    def __str__(self):
        return f"{self.name}: {self.elements}"

def draw_arrow(ax,start,end,color="black",label=None,textpos=1):
    ax.annotate("",xy=end,xycoords='data',xytext=start,textcoords='data',arrowprops=dict(arrowstyle="->",color=color),)
    if label:
        ax.text((start[0]*textpos+end[0])/(1+textpos),(start[1]*textpos+end[1])/(1+textpos)+0.2,label,fontsize=10)

def draw_set(ax,center,elements,label,radius=0.2,color="lightblue",_alpha=0.5):
    circle=patches.Circle(center,radius,edgecolor="black",facecolor=color,alpha=_alpha)
    ax.add_patch(circle)
    ax.text(center[0],center[1]+1.5,label,ha='center',fontsize=12)
    for i,elem in enumerate(elements):
        ax.text(center[0]+(i-len(elements)/2)*0.3,center[1]-2,str(elem),fontsize=10,ha='center')

def ordinals_cardinals():
    fig,ax=plt.subplots(figsize=(18,6))
    ax.set_xlim(0,24)
    ax.set_ylim(0,8)
    ax.axis('off')
    ax.set_title("Ordinals&Cardinals: Visualizing(?TOO ABSTRACT AND PLAIN!) Order",fontsize=24)
    finite_ordinals=[Ordinal("0",["φ"]),Ordinal("1",["[0],also[φ]"]),Ordinal("2",["[0,1],also[φ,[φ]]"]),Ordinal("3",["[0,1,2],also[φ,[φ],[φ,[φ]]]"]),
    ]
    positions=[(2,4),(5,4),(8,4),(11,4)]
    for i,ordinal in enumerate(finite_ordinals):
        radius=0.2
        for j in range(i+1):
            draw_set(ax,(positions[i][0], positions[i][1]),ordinal.elements if j==0 else "",ordinal.name if j==0 else "",radius,color="lightblue",_alpha=0.3+j*0.1)
            radius+=0.2
    radius=1.2
    draw_arrow(ax,(12,4),(14.6,4),color="black",label="after all N",textpos=3)
    draw_set(ax,(16,4),["[0,1,2,...]"],"ω(Cardinal)",radius=1.2,color="green",_alpha=0.7)
    draw_set(ax,(19,4),["ω"],"ω+1",radius=1.2,color="green",_alpha=0.8)
    draw_set(ax,(22,4),["ω,ω+1"],"ω+2",radius=1.2,color="green",_alpha=0.9)
    draw_arrow(ax,(17.2,4),(17.8,4),color="green")
    draw_arrow(ax,(20.2,4),(20.8,4),color="green")
    ax.text(19,6,"Cardinals(e.g.,ω,ω₁,ω₂...):\nRepresent set sizes.\nFirst ordinal of each infinite segment.",fontsize=10,ha='center',color="blue")
    ax.text(19,0.5,"Infinite Ordinals:\nω+1,ω+2,...are not cardinals\n(they are all green,same cardinality as ω!\nbut ω is the first!)",fontsize=10,ha='center',color="green")
    plt.show()

ordinals_cardinals()

