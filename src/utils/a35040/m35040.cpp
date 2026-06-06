#include "a35040/m35040.h"
QVector<double> m35040::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
