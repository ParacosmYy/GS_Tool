#include "o28014/m28014.h"
QVector<double> m28014::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
