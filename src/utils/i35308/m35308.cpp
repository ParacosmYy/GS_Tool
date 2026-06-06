#include "i35308/m35308.h"
QVector<double> m35308::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
