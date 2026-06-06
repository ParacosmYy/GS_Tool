#include "o35354/m35354.h"
QVector<double> m35354::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
