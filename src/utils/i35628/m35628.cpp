#include "i35628/m35628.h"
QVector<double> m35628::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
