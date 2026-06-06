#include "p27575/m27575.h"
QVector<double> m27575::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
