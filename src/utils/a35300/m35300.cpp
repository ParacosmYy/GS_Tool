#include "a35300/m35300.h"
QVector<double> m35300::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
