#include "n25753/m25753.h"
QVector<double> m25753::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
