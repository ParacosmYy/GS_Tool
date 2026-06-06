#include "p28575/m28575.h"
QVector<double> m28575::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
