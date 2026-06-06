#include "t7899/m7899.h"
QVector<double> m7899::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
