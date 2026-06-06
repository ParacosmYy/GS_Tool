#include "a35160/m35160.h"
QVector<double> m35160::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
