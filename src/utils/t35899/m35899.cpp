#include "t35899/m35899.h"
QVector<double> m35899::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
