#include "k35530/m35530.h"
QVector<double> m35530::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
