#include "k9930/m9930.h"
QVector<double> m9930::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
