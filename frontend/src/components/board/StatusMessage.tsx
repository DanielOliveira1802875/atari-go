import { useEffect, useState, ReactNode } from "react";

const StatusMessage = ({ children }: { children: ReactNode }) => {
  const [message, setMessage] = useState<ReactNode>(children);
  const [isTransitioning, setIsTransitioning] = useState(false);

  useEffect(() => {
    // Only trigger animation when message changes
    if (children !== message) {
      setIsTransitioning(true);

      // After fading out, update the message and fade back in
      const timer = setTimeout(() => {
        setMessage(children);
        setIsTransitioning(false);
      }, 200);

      return () => clearTimeout(timer);
    }
  }, [children, message]);

  return (
    <h1
      className={`font-bold text-md sm:text-lg md:text-xl lg:text-2xl min-h-[1.5rem] transition-all duration-200 
      ${isTransitioning ? "opacity-0 transform -translate-y-1" : "opacity-100 transform translate-y-0"}`}
    >
      {message}
    </h1>
  );
};

export default StatusMessage;
