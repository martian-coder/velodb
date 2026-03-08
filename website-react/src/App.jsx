import React, { useState, useEffect, useRef } from 'react';
import { motion, AnimatePresence } from 'framer-motion';
import { 
  Terminal, 
  Cpu, 
  Zap, 
  ShieldCheck, 
  Share2, 
  Activity, 
  ChevronRight, 
  Copy, 
  Check,
  Github,
  BookOpen,
  Layers
} from 'lucide-react';

// --- Custom Unique Logo Component ---
const VeloLogo = ({ className = "size-8" }) => (
  <svg 
    viewBox="0 0 100 100" 
    className={className} 
    fill="none" 
    xmlns="http://www.w3.org/2000/svg"
  >
    {/* Base V Shape */}
    <path 
      d="M20 20L45 85L50 90L55 85L80 20H65L50 65L35 20H20Z" 
      fill="currentColor" 
      className="text-black"
    />
    {/* Velocity Slabs / Inner Detail */}
    <path d="M38 30H62" stroke="white" strokeWidth="4" strokeLinecap="round" opacity="0.4" />
    <path d="M42 45H58" stroke="white" strokeWidth="4" strokeLinecap="round" opacity="0.6" />
    <path d="M46 60H54" stroke="white" strokeWidth="4" strokeLinecap="round" opacity="0.8" />
    {/* Outer Neon Stroke */}
    <path 
      d="M20 20L45 85L50 90L55 85L80 20" 
      stroke="currentColor" 
      strokeWidth="4" 
      strokeLinejoin="round"
      className="text-emerald-400"
    />
  </svg>
);

// --- Components ---

const Navbar = () => (
  <nav className="fixed top-0 w-full z-50 bg-black/80 backdrop-blur-md border-b border-emerald-500/20 px-6 py-4 flex justify-between items-center">
    <div className="flex items-center gap-2">
      <div className="bg-emerald-500 rounded flex items-center justify-center shadow-[0_0_20px_rgba(16,185,129,0.6)] p-1.5 transition-transform hover:scale-105 duration-300">
        <VeloLogo className="size-6 text-black" />
      </div>
      <span className="text-xl font-black tracking-tighter text-emerald-500 font-mono">VELO<span className="text-white">DB</span></span>
    </div>
    <div className="hidden md:flex gap-8 text-sm font-mono text-gray-400">
      <a href="#features" className="hover:text-emerald-400 transition-colors">FEATURES</a>
      <a href="#architecture" className="hover:text-emerald-400 transition-colors">ARCHITECTURE</a>
      <a href="#benchmarks" className="hover:text-emerald-400 transition-colors">BENCHMARKS</a>
    </div>
    <div className="flex gap-4">
      <a href="https://github.com/AmitNilajkar/VeloDB" className="p-2 border border-emerald-500/30 rounded-md hover:bg-emerald-500/10 transition-all">
        <Github className="size-5 text-emerald-500" />
      </a>
      <button className="px-4 py-2 bg-emerald-500 text-black font-mono font-bold text-xs rounded hover:bg-emerald-400 shadow-[0_0_20px_rgba(16,185,129,0.3)] transition-all">
        GET STARTED
      </button>
    </div>
  </nav>
);

const TerminalHero = () => {
  const [text, setText] = useState("");
  const fullText = "VELO_INIT --OS AMIT_NILAJKAR_CORE --V 1.0.0\n> Loading LF-BTree...\n> SAI Succinct Index [OK]\n> NXP Protocol Active...\n> Throughput: 47,000 Ops/s\n> Status: DOMINATING.";
  
  useEffect(() => {
    let i = 0;
    const interval = setInterval(() => {
      setText(fullText.slice(0, i));
      i++;
      if (i > fullText.length) clearInterval(interval);
    }, 40);
    return () => clearInterval(interval);
  }, []);

  return (
    <div className="relative group">
      <div className="absolute -inset-1 bg-emerald-500/20 rounded-lg blur opacity-75 group-hover:opacity-100 transition duration-1000 group-hover:duration-200"></div>
      <div className="relative bg-black border border-emerald-500/30 rounded-lg p-6 font-mono text-sm leading-relaxed shadow-2xl">
        <div className="flex gap-2 mb-4">
          <div className="w-3 h-3 rounded-full bg-red-500/50"></div>
          <div className="w-3 h-3 rounded-full bg-yellow-500/50"></div>
          <div className="w-3 h-3 rounded-full bg-emerald-500/50"></div>
        </div>
        <div className="text-emerald-500 whitespace-pre-wrap min-h-[140px]">
          {text}<span className="animate-pulse">_</span>
        </div>
      </div>
    </div>
  );
};

const FeatureCard = ({ icon: Icon, title, desc }) => (
  <motion.div 
    whileHover={{ y: -5 }}
    className="p-8 bg-zinc-900/50 border border-zinc-800 rounded-2xl hover:border-emerald-500/50 transition-all group"
  >
    <div className="w-12 h-12 rounded-lg bg-emerald-500/10 border border-emerald-500/20 flex items-center justify-center mb-6 group-hover:shadow-[0_0_20px_rgba(16,185,129,0.2)] transition-all">
      <Icon className="text-emerald-500 size-6" />
    </div>
    <h3 className="text-xl font-bold text-white mb-3 font-mono">{title}</h3>
    <p className="text-zinc-400 leading-relaxed text-sm">{desc}</p>
  </motion.div>
);

const LiveMetrics = () => {
  const [ops, setOps] = useState(47211);
  const [latency, setLatency] = useState(0.82);

  useEffect(() => {
    const interval = setInterval(() => {
      setOps(prev => prev + Math.floor(Math.random() * 500) - 250);
      setLatency(prev => Number((Math.random() * 0.2 + 0.7).toFixed(2)));
    }, 1000);
    return () => clearInterval(interval);
  }, []);

  return (
    <div className="grid grid-cols-1 md:grid-cols-2 gap-4">
      <div className="bg-black p-4 border border-zinc-800 rounded-xl">
        <div className="text-[10px] text-zinc-500 mb-1 font-mono uppercase tracking-widest">Throughput (Ops/s)</div>
        <div className="text-3xl font-black text-emerald-500 font-mono">{ops.toLocaleString()}</div>
      </div>
      <div className="bg-black p-4 border border-zinc-800 rounded-xl">
        <div className="text-[10px] text-zinc-500 mb-1 font-mono uppercase tracking-widest">p99 Latency (ms)</div>
        <div className="text-3xl font-black text-emerald-500 font-mono">{latency}</div>
      </div>
    </div>
  );
};

// --- Main App ---

export default function App() {
  const [copied, setCopied] = useState(false);
  const installCmd = "git clone https://github.com/AmitNilajkar/VeloDB";

  const copyToClipboard = () => {
    navigator.clipboard.writeText(installCmd);
    setCopied(true);
    setTimeout(() => setCopied(false), 2000);
  };

  return (
    <div className="min-h-screen bg-black text-white selection:bg-emerald-500/30 font-sans overflow-x-hidden">
      {/* Background Effects */}
      <div className="fixed inset-0 pointer-events-none">
        <div className="absolute top-0 left-1/2 -translate-x-1/2 w-full h-[500px] bg-emerald-500/5 blur-[120px] rounded-full opacity-50"></div>
        <div className="absolute inset-0 bg-[url('https://grainy-gradients.vercel.app/noise.svg')] opacity-20 contrast-150"></div>
        <div className="absolute inset-0 pointer-events-none bg-[linear-gradient(rgba(18,16,16,0)_50%,rgba(0,0,0,0.25)_50%),linear-gradient(90deg,rgba(255,0,0,0.06),rgba(0,255,0,0.02),rgba(0,0,255,0.06))] z-[100] bg-[length:100%_2px,3px_100%]"></div>
      </div>

      <Navbar />

      <main className="relative pt-32 px-6 max-w-7xl mx-auto">
        
        {/* Hero Section */}
        <section className="grid lg:grid-cols-2 gap-16 items-center py-20">
          <div>
            <motion.div 
              initial={{ opacity: 0, x: -20 }}
              animate={{ opacity: 1, x: 0 }}
              className="inline-flex items-center gap-2 px-3 py-1 rounded-full border border-emerald-500/30 bg-emerald-500/5 text-emerald-400 text-xs font-mono font-bold mb-6"
            >
              <Zap className="size-3 fill-emerald-400" />
              BEYOND HTTPS PROTOCOL V1.0.0
            </motion.div>
            <motion.h1 
              initial={{ opacity: 0, y: 20 }}
              animate={{ opacity: 1, y: 0 }}
              transition={{ delay: 0.1 }}
              className="text-6xl md:text-8xl font-black tracking-tight leading-[0.9] mb-8"
            >
              FAST. <br />
              <span className="text-emerald-500">LOCK-FREE.</span> <br />
              DURABLE.
            </motion.h1>
            <motion.p 
              initial={{ opacity: 0, y: 20 }}
              animate={{ opacity: 1, y: 0 }}
              transition={{ delay: 0.2 }}
              className="text-zinc-400 text-lg md:text-xl max-w-lg mb-10 leading-relaxed"
            >
              The world's most unique in-memory database. 
              Engineered with O(log n) efficiency and the custom Nexus Protocol for sub-microsecond mastery.
            </motion.p>
            
            <motion.div 
              initial={{ opacity: 0, y: 20 }}
              animate={{ opacity: 1, y: 0 }}
              transition={{ delay: 0.3 }}
              className="flex flex-col sm:flex-row gap-4"
            >
              <div className="flex-1 bg-zinc-900 border border-zinc-800 rounded-lg p-1 flex items-center justify-between group">
                <code className="px-4 text-emerald-500 font-mono text-xs">{installCmd}</code>
                <button 
                  onClick={copyToClipboard}
                  className="p-2 hover:bg-zinc-800 rounded-md transition-colors"
                >
                  {copied ? <Check className="size-4 text-emerald-400" /> : <Copy className="size-4 text-zinc-500" />}
                </button>
              </div>
              <button className="px-8 py-3 bg-white text-black font-bold rounded-lg hover:bg-zinc-200 transition-all flex items-center justify-center gap-2">
                Launch Docs <ChevronRight className="size-4" />
              </button>
            </motion.div>
          </div>

          <motion.div
            initial={{ opacity: 0, scale: 0.9 }}
            animate={{ opacity: 1, scale: 1 }}
            transition={{ delay: 0.4 }}
          >
            <TerminalHero />
          </motion.div>
        </section>

        {/* Live Status Bar */}
        <motion.section 
          initial={{ opacity: 0 }}
          whileInView={{ opacity: 1 }}
          className="border-y border-zinc-800 py-12 mb-32"
        >
          <div className="grid md:grid-cols-3 gap-12 items-center text-center md:text-left">
            <div>
              <div className="text-emerald-500/50 text-xs font-mono uppercase tracking-[0.2em] mb-4">Live Performance</div>
              <LiveMetrics />
            </div>
            <div className="md:border-l border-zinc-800 md:pl-12">
              <div className="text-3xl font-bold mb-1 font-mono">2.5M+</div>
              <div className="text-zinc-500 text-sm font-mono uppercase tracking-widest">Theoretical Ops Cap</div>
            </div>
            <div className="md:border-l border-zinc-800 md:pl-12">
              <div className="text-3xl font-bold mb-1 font-mono">0ms</div>
              <div className="text-zinc-500 text-sm font-mono uppercase tracking-widest">Wait-Time for Reads</div>
            </div>
          </div>
        </motion.section>

        {/* Features */}
        <section id="features" className="py-20">
          <div className="mb-20 text-center">
            <h2 className="text-4xl md:text-5xl font-black mb-6">BUILT DIFFERENT.</h2>
            <p className="text-zinc-500 max-w-2xl mx-auto">VeloDB bypasses traditional database bottlenecks with architectural innovations designed for modern many-core hardware.</p>
          </div>

          <div className="grid md:grid-cols-2 lg:grid-cols-3 gap-6">
            <FeatureCard 
              icon={Cpu}
              title="Lock-Free B-Tree"
              desc="Immutable slabs and atomic root swaps enable wait-free reads. No mutexes. No bottlenecks. Just raw speed."
            />
            <FeatureCard 
              icon={Zap}
              title="Nexus Protocol (NXP)"
              desc="A custom binary wire protocol that cuts out HTTPS overhead, optimized for massive data ingestion."
            />
            <FeatureCard 
              icon={Layers}
              title="SAI Succinct Index"
              desc="Elias-Fano based indexing provides O(1) rank/select operations for lightning fast range queries."
            />
            <FeatureCard 
              icon={ShieldCheck}
              title="Checksummed WAL"
              desc="Enterprise-grade durability with CRC32 integrity checks. Automatically recovers after system failure."
            />
            <FeatureCard 
              icon={Share2}
              title="Python Bindings"
              desc="Seamlessly integrate with data pipelines. High-speed C++ engine exposed via native Python API."
            />
            <FeatureCard 
              icon={Activity}
              title="Zero-Copy Processing"
              desc="Data is mapped directly to memory structures, eliminating the cost of serialization."
            />
          </div>
        </section>

        {/* Architecture Spotlight */}
        <section id="architecture" className="py-40">
          <div className="bg-zinc-900/40 border border-zinc-800 rounded-3xl p-8 md:p-16 flex flex-col lg:flex-row gap-16 items-center overflow-hidden relative">
            <div className="absolute top-0 right-0 w-[400px] h-[400px] bg-emerald-500/10 blur-[100px] rounded-full"></div>
            
            <div className="flex-1">
              <h2 className="text-3xl md:text-4xl font-black mb-8 font-mono tracking-tighter">
                DEEP DIVE: <span className="text-emerald-500 italic underline">COW B-TREE</span>
              </h2>
              <ul className="space-y-6">
                {[
                  "Path-Copying ensures old data is never mutated during writes.",
                  "Readers always see a globally consistent snapshot without locks.",
                  "O(log N) insertion complexity scales to millions of records.",
                  "Self-optimizing node splits keep the structure perfectly balanced."
                ].map((item, i) => (
                  <li key={i} className="flex gap-4 text-zinc-300">
                    <div className="mt-1 text-emerald-500"><ChevronRight className="size-5" /></div>
                    <span>{item}</span>
                  </li>
                ))}
              </ul>
              <button className="mt-12 px-6 py-3 border border-emerald-500/30 text-emerald-500 rounded-lg hover:bg-emerald-500/10 transition-all font-mono text-sm">
                READ THE WHITE PAPER
              </button>
            </div>

            <div className="flex-1 w-full relative">
              <div className="aspect-square bg-black border border-zinc-800 rounded-2xl flex items-center justify-center p-8 overflow-hidden">
                <div className="w-full space-y-4 opacity-70 scale-110 rotate-12">
                   {[1,2,3,4,5].map(i => (
                     <div key={i} className={`h-1 bg-emerald-500/40 rounded w-full`} style={{ opacity: 1 - i*0.15, marginLeft: i*20 }}></div>
                   ))}
                   <div className="flex justify-between px-10">
                    <div className="w-4 h-4 rounded-full bg-emerald-500 shadow-[0_0_15px_rgba(16,185,129,0.8)]"></div>
                    <div className="w-4 h-4 rounded-full bg-emerald-500 shadow-[0_0_15px_rgba(16,185,129,0.8)]"></div>
                   </div>
                   <div className="h-2 bg-emerald-500/20 rounded-full w-2/3 mx-auto"></div>
                </div>
              </div>
            </div>
          </div>
        </section>

        {/* CTA */}
        <section className="py-40 text-center relative">
          <div className="absolute top-1/2 left-1/2 -translate-x-1/2 -translate-y-1/2 w-full max-w-4xl h-80 bg-emerald-500/20 blur-[140px] rounded-full pointer-events-none"></div>
          <h2 className="text-5xl md:text-7xl font-black mb-8">READY TO BREAK <br /> THE LIMITS?</h2>
          <p className="text-zinc-400 max-w-xl mx-auto mb-12 text-lg italic font-mono uppercase tracking-widest">
            "Software is either fast or wrong. We chose fast." - AMIT NILAJKAR
          </p>
          <div className="flex flex-col sm:flex-row gap-6 justify-center">
            <button className="px-10 py-4 bg-emerald-500 text-black font-black rounded-full hover:bg-emerald-400 transition-all shadow-[0_0_30px_rgba(16,185,129,0.4)]">
              DOWNLOAD V1.0.0
            </button>
            <button className="px-10 py-4 border border-zinc-700 rounded-full hover:border-white transition-all font-bold">
              STAR ON GITHUB
            </button>
          </div>
        </section>

      </main>

      <footer className="border-t border-zinc-800 py-12 px-6 mt-40">
        <div className="max-w-7xl mx-auto flex flex-col md:row justify-between items-center gap-8">
          <div className="flex items-center gap-2">
            <div className="bg-zinc-900 border border-zinc-800 rounded p-1.5 flex items-center justify-center">
              <VeloLogo className="size-5 text-emerald-500" />
            </div>
            <span className="text-lg font-black tracking-tighter text-zinc-300 font-mono italic">VELODB</span>
          </div>
          <p className="text-zinc-600 text-sm font-mono tracking-tighter">
            © 2026 AMIT NILAJKAR. PROTECTED BY NXP AND TRADEMARK LAW.
          </p>
          <div className="flex gap-6 text-zinc-500 text-xs font-mono">
             <a href="#" className="hover:text-white">LICENSING</a>
             <a href="#" className="hover:text-white">CONTRIBUTING</a>
             <a href="#" className="hover:text-white">MANIFESTO</a>
          </div>
        </div>
      </footer>
    </div>
  );
}
