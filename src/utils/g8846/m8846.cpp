#include "g8846/m8846.h"
QVector<double> m8846::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
