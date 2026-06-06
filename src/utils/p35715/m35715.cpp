#include "p35715/m35715.h"
QVector<double> m35715::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
