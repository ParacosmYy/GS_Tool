#include "t35799/m35799.h"
QVector<double> m35799::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
