{{/*
Развернуть имя чарта.
*/}}
{{- define "honeycomb.name" -}}
{{- default .Chart.Name .Values.nameOverride | trunc 63 | trimSuffix "-" }}
{{- end }}

{{/*
Создать полностью квалифицированное имя приложения.
*/}}
{{- define "honeycomb.fullname" -}}
{{- if .Values.fullnameOverride }}
{{- .Values.fullnameOverride | trunc 63 | trimSuffix "-" }}
{{- else }}
{{- $name := default .Chart.Name .Values.nameOverride }}
{{- if contains $name .Release.Name }}
{{- .Release.Name | trunc 63 | trimSuffix "-" }}
{{- else }}
{{- printf "%s-%s" .Release.Name $name | trunc 63 | trimSuffix "-" }}
{{- end }}
{{- end }}
{{- end }}

{{/*
Метки для всех ресурсов.
*/}}
{{- define "honeycomb.labels" -}}
helm.sh/chart: {{ include "honeycomb.name" . }}-{{ .Chart.Version | replace "+" "_" }}
{{ include "honeycomb.selectorLabels" . }}
{{- if .Chart.AppVersion }}
app.kubernetes.io/version: {{ .Chart.AppVersion | quote }}
{{- end }}
app.kubernetes.io/managed-by: {{ .Release.Service }}
app.kubernetes.io/part-of: {{ include "honeycomb.name" . }}
{{- end }}

{{/*
Селекторные метки.
*/}}
{{- define "honeycomb.selectorLabels" -}}
app.kubernetes.io/name: {{ include "honeycomb.name" . }}
app.kubernetes.io/instance: {{ .Release.Name }}
{{- end }}

{{/*
Добавить метку региона (для географических кластеров).
*/}}
{{- define "honeycomb.regionLabel" -}}
{{- if .Values.region }}
region: {{ .Values.region }}
{{- else }}
region: global
{{- end }}
{{- end }}

{{/*
Создать имя для секрета с TLS-сертификатом.
*/}}
{{- define "honeycomb.tlsSecretName" -}}
{{- if .Values.ingress.tlsSecretName }}
{{- .Values.ingress.tlsSecretName }}
{{- else }}
{{- printf "%s-tls" (include "honeycomb.fullname" .) }}
{{- end }}
{{- end }}