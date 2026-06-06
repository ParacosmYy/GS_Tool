#include "b15721/m15721.h"
QVector<double> m15721::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
