#include "k35150/m35150.h"
QVector<double> m35150::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
