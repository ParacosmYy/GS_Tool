#include "f17685/m17685.h"
QVector<double> m17685::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
