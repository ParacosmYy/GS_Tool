#include "k35550/m35550.h"
QVector<double> m35550::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
