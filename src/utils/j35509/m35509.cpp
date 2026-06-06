#include "j35509/m35509.h"
QVector<double> m35509::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
