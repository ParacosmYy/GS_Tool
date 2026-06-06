#include "k9770/m9770.h"
QVector<double> m9770::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
