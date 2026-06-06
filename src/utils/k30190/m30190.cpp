#include "k30190/m30190.h"
QVector<double> m30190::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
