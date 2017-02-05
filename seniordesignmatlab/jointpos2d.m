%Sragvi Tirumala
%Senior Design
%25 January, 2017
%Joint position plotting over time
close all

for i=1:400
    if JOINT_HEADx(i,1)== 0
    else
    %Plot joint positions in 3 dimensions
    plot(JOINT_HEADx(i,1),JOINT_HEADy(i,1),'o','MarkerFaceColor','r');
    hold on
    plot(JOINT_SHOULDER_LEFTx(i,1),JOINT_SHOULDER_LEFTy(i,1),'*','MarkerFaceColor','b');
    hold on
    plot(JOINT_SHOULDER_RIGHTx(i,1),JOINT_SHOULDER_RIGHTy(i,1),'*','MarkerFaceColor','g');
    hold on
    plot(JOINT_HAND_LEFTx(i,1),JOINT_HAND_LEFTy(i,1),'v','MarkerFaceColor','b');
    hold on
    plot(JOINT_HAND_RIGHTx(i,1),JOINT_HAND_RIGHTy(i,1),'v','MarkerFaceColor','g');
    hold on
    plot(JOINT_SPINE_MIDx(i,1),JOINT_SPINE_MIDy(i,1),'s','MarkerFaceColor','r'); 
    hold on
    plot(CenterMassx(i,1), CenterMassy(i,1),'o');
    
    
    %plot text next to each point
    text(JOINT_HEADx(i,1),JOINT_HEADy(i,1),'\leftarrow Head');
    text(JOINT_SHOULDER_LEFTx(i,1),JOINT_SHOULDER_LEFTy(i,1),'\leftarrow LeftSho');
    text(JOINT_SHOULDER_RIGHTx(i,1),JOINT_SHOULDER_RIGHTy(i,1),'\leftarrow RightSho');
    text(JOINT_HAND_LEFTx(i,1),JOINT_HAND_LEFTy(i,1),'\leftarrow LeftHand');
    text(JOINT_HAND_RIGHTx(i,1),JOINT_HAND_RIGHTy(i,1),'\leftarrow RightHand');
    text(JOINT_SPINE_MIDx(i,1),JOINT_SPINE_MIDy(i,1),'\leftarrow Spine');
    text(CenterMassx(i,1),CenterMassy(i,1),'\leftarrow CM');
    
    
    %plot specifics
    title('Joint positions over time');
    axis([0 300 0 300])
    axis ij
    xlabel('x')
    ylabel('y')
    grid on
    pause(.2)
    hold off
    end
end