#include "p35835/m35835.h"
QVector<double> m35835::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
