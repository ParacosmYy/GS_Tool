#include "p35135/m35135.h"
QVector<double> m35135::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
