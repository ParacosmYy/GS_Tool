#include "c25022/m25022.h"
QVector<double> m25022::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
