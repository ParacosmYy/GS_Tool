#include "q24576/m24576.h"
QVector<double> m24576::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
