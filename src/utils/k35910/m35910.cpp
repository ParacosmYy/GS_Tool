#include "k35910/m35910.h"
QVector<double> m35910::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
