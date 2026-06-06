#include "f35645/m35645.h"
QVector<double> m35645::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
