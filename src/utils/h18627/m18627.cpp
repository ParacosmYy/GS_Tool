#include "h18627/m18627.h"
QVector<double> m18627::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
