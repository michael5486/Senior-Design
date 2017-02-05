%Sragvi Tirumala
%Senior Design
%25 January, 2017
%Joint position plotting over time
close all

for i=1:400
    %Plot joint positions in 3 dimensions
    plot3(JOINT_HEADy(i,1),JOINT_HEADx(i,1),JOINT_HEADz(i,1),'o','MarkerFaceColor','r');
    hold on
    plot3(JOINT_SHOULDER_LEFTy(i,1),JOINT_SHOULDER_LEFTx(i,1),JOINT_SHOULDER_LEFTz(i,1),'*','MarkerFaceColor','b');
    hold on
    plot3(JOINT_SHOULDER_RIGHTy(i,1),JOINT_SHOULDER_RIGHTx(i,1),JOINT_SHOULDER_RIGHTz(i,1),'*','MarkerFaceColor','g');
    hold on
    plot3(JOINT_HAND_LEFTy(i,1),JOINT_HAND_LEFTx(i,1),JOINT_HAND_LEFTz(i,1),'v','MarkerFaceColor','b');
    hold on
    plot3(JOINT_HAND_RIGHTy(i,1),JOINT_HAND_RIGHTx(i,1),JOINT_HAND_RIGHTz(i,1),'v','MarkerFaceColor','g');
    hold on
    plot3(JOINT_SPINE_MIDy(i,1),JOINT_SPINE_MIDx(i,1),JOINT_SPINE_MIDz(i,1),'s','MarkerFaceColor','r');
   
    %plot text next to each point
    text(JOINT_HEADy(i,1),JOINT_HEADx(i,1),JOINT_HEADz(i,1),'\leftarrow Head');
    text(JOINT_SHOULDER_LEFTy(i,1),JOINT_SHOULDER_LEFTx(i,1),JOINT_SHOULDER_LEFTz(i,1),'\leftarrow LeftSho');
    text(JOINT_SHOULDER_RIGHTy(i,1),JOINT_SHOULDER_RIGHTx(i,1),JOINT_SHOULDER_RIGHTz(i,1),'\leftarrow RightSho');
    text(JOINT_HAND_LEFTy(i,1),JOINT_HAND_LEFTx(i,1),JOINT_HAND_LEFTz(i,1),'\leftarrow LeftHand');
    text(JOINT_HAND_RIGHTy(i,1),JOINT_HAND_RIGHTx(i,1),JOINT_HAND_RIGHTz(i,1),'\leftarrow RightHand');
    text(JOINT_SPINE_MIDy(i,1),JOINT_SPINE_MIDx(i,1),JOINT_SPINE_MIDz(i,1),'\leftarrow Spine');
    
    
    %plot specifics
    title('Joint positions over time');
    axis([-500 500 -500 500 0 2000])
    xlabel('x')
    ylabel('y')
    zlabel('z')
    grid on
    pause(.4)
    hold off
end