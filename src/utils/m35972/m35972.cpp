#include "m35972/m35972.h"
QVector<double> m35972::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
