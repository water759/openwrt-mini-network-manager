<template>
  <div class="page-card">
    <div class="page-card-title">配置管理</div>
    <el-form label-width="120px" style="max-width: 520px">
      <el-form-item label="监听网卡">
        <el-input model-value="br-lan" />
      </el-form-item>
      <el-form-item label="BPF 过滤器">
        <el-input model-value="tcp" />
      </el-form-item>
      <el-form-item label="JSON 输出路径">
        <el-input model-value="/tmp/netmon/stats.json" />
      </el-form-item>
      <el-form-item label="刷新间隔">
        <el-select model-value="2" style="width: 120px">
          <el-option label="1 秒" :value="1" />
          <el-option label="2 秒" :value="2" />
          <el-option label="5 秒" :value="5" />
        </el-select>
      </el-form-item>
      <el-form-item label="API 地址">
        <el-input model-value="http://192.168.1.1:8080" />
      </el-form-item>
      <el-form-item>
        <el-button type="primary">保存配置</el-button>
        <el-button>恢复默认</el-button>
      </el-form-item>
    </el-form>
  </div>
</template>

<script setup>
</script>
