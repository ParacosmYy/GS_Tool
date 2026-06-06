#include "j35709/m35709.h"
QVector<double> m35709::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
