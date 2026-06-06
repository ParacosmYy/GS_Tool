#include "m25672/m25672.h"
QVector<double> m25672::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
