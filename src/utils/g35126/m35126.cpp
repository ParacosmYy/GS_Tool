#include "g35126/m35126.h"
QVector<double> m35126::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
