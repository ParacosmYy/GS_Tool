#include "n25153/m25153.h"
QVector<double> m25153::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
