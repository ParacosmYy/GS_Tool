#include "d7923/m7923.h"
QVector<double> m7923::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
