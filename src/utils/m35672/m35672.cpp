#include "m35672/m35672.h"
QVector<double> m35672::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
