import { useState, useEffect } from 'react';
import { Activity, Cpu, HardDrive, Network, Server, TrendingUp } from 'lucide-react';
import { LineChart, Line, AreaChart, Area, XAxis, YAxis, CartesianGrid, Tooltip, Legend, ResponsiveContainer } from 'recharts';

export default function MonitoringDashboard() {
  const [metrics, setMetrics] = useState(null);
  const [history, setHistory] = useState([]);
  const [isConnected, setIsConnected] = useState(false);
  const [apiUrl, setApiUrl] = useState('http://localhost:9090');
  const [error, setError] = useState(null);

  useEffect(() => {
    const fetchMetrics = async () => {
      try {
        const response = await fetch(`${apiUrl}/metrics`);
        if (!response.ok) throw new Error('Failed to fetch metrics');
        
        const data = await response.json();
        setMetrics(data);
        setIsConnected(true);
        setError(null);

        // Add to history - keep last 20 data points
        setHistory(prev => {
          const newHistory = [...prev, {
            time: new Date().toLocaleTimeString(),
            cpu: data.cpu_usage,
            memory: data.memory_usage_kb / 1024,
            disk_read: data.disk.bytes_read / 1024 / 1024,
            disk_write: data.disk.bytes_written / 1024 / 1024,
            net_recv: data.network.bytes_received / 1024 / 1024,
            net_sent: data.network.bytes_sent / 1024 / 1024
          }];
          return newHistory.slice(-20);
        });
      } catch (err) {
        setIsConnected(false);
        setError(err.message);
      }
    };

    fetchMetrics();
    const interval = setInterval(fetchMetrics, 5000);
    return () => clearInterval(interval);
  }, [apiUrl]);

  const formatBytes = (bytes) => {
    if (bytes === 0) return '0 B';
    const k = 1024;
    const sizes = ['B', 'KB', 'MB', 'GB'];
    const i = Math.floor(Math.log(bytes) / Math.log(k));
    return Math.round(bytes / Math.pow(k, i) * 100) / 100 + ' ' + sizes[i];
  };

  const MetricCard = ({ title, value, icon: Icon, color, suffix = '' }) => (
    <div className="bg-dark-card rounded-lg shadow-card hover:shadow-card-hover transition-shadow p-6 border-l-4" style={{ borderColor: color }}>
      <div className="flex items-center justify-between">
        <div>
          <p className="text-gray-400 text-sm font-medium">{title}</p>
          <p className="text-3xl font-bold mt-2 text-white">{value}{suffix}</p>
        </div>
        <Icon className="w-12 h-12 opacity-20" style={{ color }} />
      </div>
    </div>
  );

  if (error) {
    return (
      <div className="min-h-screen bg-dashboard-dark flex items-center justify-center p-4">
        <div className="bg-dark-card rounded-lg shadow-card-hover p-8 max-w-md w-full border border-dark-border">
          <div className="text-center">
            <Server className="w-16 h-16 text-red-500 mx-auto mb-4" />
            <h2 className="text-2xl font-bold text-white mb-2">Connection Error</h2>
            <p className="text-gray-400 mb-4">{error}</p>
            <p className="text-sm text-gray-500 mb-4">Make sure your monitoring server is running:</p>
            <div className="bg-dark-bg p-3 rounded text-left text-xs font-mono mb-4 text-gray-300 border border-dark-border">
              ./microservice_demo
            </div>
            <input
              type="text"
              value={apiUrl}
              onChange={(e) => setApiUrl(e.target.value)}
              className="w-full px-4 py-2 bg-dark-bg border border-dark-border rounded mb-2 text-white placeholder-gray-500 focus:outline-none focus:border-primary-500"
              placeholder="API URL"
            />
            <p className="text-xs text-gray-500">Default: http://localhost:9090</p>
          </div>
        </div>
      </div>
    );
  }

  if (!metrics) {
    return (
      <div className="min-h-screen bg-dashboard-dark flex items-center justify-center">
        <div className="text-center">
          <Activity className="w-16 h-16 text-primary-500 mx-auto mb-4 animate-pulse" />
          <p className="text-xl text-gray-400">Loading metrics...</p>
        </div>
      </div>
    );
  }

  return (
    <div className="min-h-screen bg-dashboard-dark p-6">
      {/* Header */}
      <div className="max-w-7xl mx-auto mb-6">
        <div className="flex items-center justify-between">
          <div>
            <h1 className="text-3xl font-bold text-white">Performance Monitor</h1>
            <p className="text-gray-400">Real-time system metrics</p>
          </div>
          <div className="flex items-center gap-2">
            <div className={`w-3 h-3 rounded-full ${isConnected ? 'bg-green-500' : 'bg-red-500'} animate-pulse`}></div>
            <span className="text-sm text-gray-400">{isConnected ? 'Connected' : 'Disconnected'}</span>
          </div>
        </div>
      </div>

      {/* Metric Cards */}
      <div className="max-w-7xl mx-auto grid grid-cols-1 md:grid-cols-2 lg:grid-cols-4 gap-6 mb-6">
        <MetricCard
          title="CPU Usage"
          value={metrics.cpu_usage.toFixed(1)}
          suffix="%"
          icon={Cpu}
          color="#3b82f6"
        />
        <MetricCard
          title="Memory"
          value={(metrics.memory_usage_kb / 1024).toFixed(0)}
          suffix=" MB"
          icon={Server}
          color="#10b981"
        />
        <MetricCard
          title="Processes"
          value={metrics.processes}
          icon={Activity}
          color="#f59e0b"
        />
        <MetricCard
          title="Load (1m)"
          value={metrics.load_average['1min'].toFixed(2)}
          icon={TrendingUp}
          color="#8b5cf6"
        />
      </div>

      {/* Charts */}
      <div className="max-w-7xl mx-auto grid grid-cols-1 lg:grid-cols-2 gap-6 mb-6">
        {/* CPU Chart */}
        <div className="bg-dark-card rounded-lg shadow-card p-6 border border-dark-border">
          <h3 className="text-lg font-semibold mb-4 flex items-center gap-2 text-white">
            <Cpu className="w-5 h-5 text-blue-500" />
            CPU Usage Over Time
          </h3>
          <ResponsiveContainer width="100%" height={200}>
            <AreaChart data={history}>
              <CartesianGrid strokeDasharray="3 3" stroke="#334155" />
              <XAxis dataKey="time" tick={{ fontSize: 12, fill: '#94a3b8' }} />
              <YAxis tick={{ fontSize: 12, fill: '#94a3b8' }} />
              <Tooltip 
                contentStyle={{ backgroundColor: '#1e293b', border: '1px solid #334155', borderRadius: '0.5rem' }}
                labelStyle={{ color: '#f1f5f9' }}
              />
              <Area type="monotone" dataKey="cpu" stroke="#3b82f6" fill="#3b82f6" fillOpacity={0.3} />
            </AreaChart>
          </ResponsiveContainer>
        </div>

        {/* Memory Chart */}
        <div className="bg-dark-card rounded-lg shadow-card p-6 border border-dark-border">
          <h3 className="text-lg font-semibold mb-4 flex items-center gap-2 text-white">
            <Server className="w-5 h-5 text-green-500" />
            Memory Usage (MB)
          </h3>
          <ResponsiveContainer width="100%" height={200}>
            <AreaChart data={history}>
              <CartesianGrid strokeDasharray="3 3" stroke="#334155" />
              <XAxis dataKey="time" tick={{ fontSize: 12, fill: '#94a3b8' }} />
              <YAxis tick={{ fontSize: 12, fill: '#94a3b8' }} />
              <Tooltip 
                contentStyle={{ backgroundColor: '#1e293b', border: '1px solid #334155', borderRadius: '0.5rem' }}
                labelStyle={{ color: '#f1f5f9' }}
              />
              <Area type="monotone" dataKey="memory" stroke="#10b981" fill="#10b981" fillOpacity={0.3} />
            </AreaChart>
          </ResponsiveContainer>
        </div>

        {/* Disk I/O Chart */}
        <div className="bg-dark-card rounded-lg shadow-card p-6 border border-dark-border">
          <h3 className="text-lg font-semibold mb-4 flex items-center gap-2 text-white">
            <HardDrive className="w-5 h-5 text-purple-500" />
            Disk I/O (MB)
          </h3>
          <ResponsiveContainer width="100%" height={200}>
            <LineChart data={history}>
              <CartesianGrid strokeDasharray="3 3" stroke="#334155" />
              <XAxis dataKey="time" tick={{ fontSize: 12, fill: '#94a3b8' }} />
              <YAxis tick={{ fontSize: 12, fill: '#94a3b8' }} />
              <Tooltip 
                contentStyle={{ backgroundColor: '#1e293b', border: '1px solid #334155', borderRadius: '0.5rem' }}
                labelStyle={{ color: '#f1f5f9' }}
              />
              <Legend wrapperStyle={{ color: '#94a3b8' }} />
              <Line type="monotone" dataKey="disk_read" stroke="#8b5cf6" name="Read" />
              <Line type="monotone" dataKey="disk_write" stroke="#ec4899" name="Write" />
            </LineChart>
          </ResponsiveContainer>
        </div>

        {/* Network Chart */}
        <div className="bg-dark-card rounded-lg shadow-card p-6 border border-dark-border">
          <h3 className="text-lg font-semibold mb-4 flex items-center gap-2 text-white">
            <Network className="w-5 h-5 text-orange-500" />
            Network Traffic (MB)
          </h3>
          <ResponsiveContainer width="100%" height={200}>
            <LineChart data={history}>
              <CartesianGrid strokeDasharray="3 3" stroke="#334155" />
              <XAxis dataKey="time" tick={{ fontSize: 12, fill: '#94a3b8' }} />
              <YAxis tick={{ fontSize: 12, fill: '#94a3b8' }} />
              <Tooltip 
                contentStyle={{ backgroundColor: '#1e293b', border: '1px solid #334155', borderRadius: '0.5rem' }}
                labelStyle={{ color: '#f1f5f9' }}
              />
              <Legend wrapperStyle={{ color: '#94a3b8' }} />
              <Line type="monotone" dataKey="net_recv" stroke="#10b981" name="Received" />
              <Line type="monotone" dataKey="net_sent" stroke="#f59e0b" name="Sent" />
            </LineChart>
          </ResponsiveContainer>
        </div>
      </div>

      {/* Load Average & Network/Disk Stats */}
      <div className="max-w-7xl mx-auto grid grid-cols-1 lg:grid-cols-2 gap-6">
        <div className="bg-dark-card rounded-lg shadow-card p-6 border border-dark-border">
          <h3 className="text-lg font-semibold mb-4 text-white">Load Average</h3>
          <div className="space-y-3">
            <div className="flex justify-between items-center">
              <span className="text-gray-400">1 minute</span>
              <span className="font-bold text-lg text-white">{metrics.load_average['1min'].toFixed(2)}</span>
            </div>
            <div className="flex justify-between items-center">
              <span className="text-gray-400">5 minutes</span>
              <span className="font-bold text-lg text-white">{metrics.load_average['5min'].toFixed(2)}</span>
            </div>
            <div className="flex justify-between items-center">
              <span className="text-gray-400">15 minutes</span>
              <span className="font-bold text-lg text-white">{metrics.load_average['15min'].toFixed(2)}</span>
            </div>
          </div>
        </div>

        <div className="bg-dark-card rounded-lg shadow-card p-6 border border-dark-border">
          <h3 className="text-lg font-semibold mb-4 text-white">Current I/O Stats</h3>
          <div className="space-y-3">
            <div className="flex justify-between items-center">
              <span className="text-gray-400">Disk Read</span>
              <span className="font-bold text-lg text-white">{formatBytes(metrics.disk.bytes_read)}</span>
            </div>
            <div className="flex justify-between items-center">
              <span className="text-gray-400">Disk Write</span>
              <span className="font-bold text-lg text-white">{formatBytes(metrics.disk.bytes_written)}</span>
            </div>
            <div className="flex justify-between items-center">
              <span className="text-gray-400">Network Received</span>
              <span className="font-bold text-lg text-white">{formatBytes(metrics.network.bytes_received)}</span>
            </div>
            <div className="flex justify-between items-center">
              <span className="text-gray-400">Network Sent</span>
              <span className="font-bold text-lg text-white">{formatBytes(metrics.network.bytes_sent)}</span>
            </div>
          </div>
        </div>
      </div>
    </div>
  );
}