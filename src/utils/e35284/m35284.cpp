#include "e35284/m35284.h"
QVector<double> m35284::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
