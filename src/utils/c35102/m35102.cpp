#include "c35102/m35102.h"
QVector<double> m35102::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
