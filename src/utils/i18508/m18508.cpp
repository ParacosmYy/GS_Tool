#include "i18508/m18508.h"
QVector<double> m18508::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
