#include "k35730/m35730.h"
QVector<double> m35730::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
