#include "e35444/m35444.h"
QVector<double> m35444::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
