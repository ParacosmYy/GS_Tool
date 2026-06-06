#include "f28625/m28625.h"
QVector<double> m28625::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
